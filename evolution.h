#include <vector>
#include <random>
#include <algorithm>

using namespace std;

#ifndef GENOME_EVOLUTION_H
#define GENOME_EVOLUTION_H

// Define the Scale enum outside of the class definition
enum Scale {Minor, Major};



class Agent {
public:
    vector<int> notes;

    // Pass the Scale enum as an argument instead of an int
    explicit Agent(
            vector<int> target,
            Scale scale = Minor,
            vector<int> notes = {}
    ) : target(target), scale(scale), notes(target.size())
    {
        if (notes.empty()) {
            for (int i = 0; i < notes.size(); ++i) {
                // Use the appropriate notes vector based on the Scale enum
                notes.push_back(scale == Minor ? rand() % 7 : rand() % 12);
            }
        }
        this->notes = notes;
    }
    ~Agent() = default;

    int operator[](int i) const {
        return notes[i];
    }

    int size() const {
        return notes.size();
    }

    bool operator==(const Agent &other) const {
        if (notes.size() != other.notes.size()) {
            return false;
        }
        for (int i = 0; i < notes.size(); ++i) {
            if (notes[i] != other.notes[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator<=(const Agent &other) const {
        return evaluate() >= other.evaluate();
    }

    bool operator<(const Agent &other) const {
        return evaluate() > other.evaluate();
    }

    bool operator>=(const Agent &other) const {
        return evaluate() <= other.evaluate();
    }

    bool operator>(const Agent &other) const {
        return evaluate() < other.evaluate();
    }

    void set_target(vector<int> _target) {
        this->target = _target;
    }

    Agent reproduce(const Agent &partner) const {
        vector<int> crossover_mask(notes.size());
        for (int i = 0; i < notes.size(); ++i) {
            crossover_mask[i] = rand() % 2;
        }
        vector<int> new_notes(notes.size());
        for (int i = 0; i < notes.size(); ++i) {
            new_notes[i] = crossover_mask[i] ? partner.notes[i] : notes[i];
        }
        return Agent(target, scale, new_notes);
    }

    int evaluate() const {
        int score = 0.f;
        for (int i = 0; i < notes.size() - 1; ++i) {
            if (make_pair(notes[i], notes[i + 1]) == make_pair(target[i], target[i + 1])) {
                score += 1.f;
            }
        }
        return score;
    }

    void mutate() {
        // Generate a random index within the range of the vector's indices
        uniform_int_distribution<> distribution = distr(0, notes.size() - 1);
        int note_to_mutate = distribution(gen);
        vector<int> available_notes;
        for (int note: scale == Minor ? vector<int>{0, 2, 3, 5, 7, 8, 10} : vector<int>{0, 2, 4, 5, 7, 9, 11}) {
            if (note != note_to_mutate) {
                available_notes.push_back(note);
            }
        }
        notes[rand() % notes.size()] = available_notes[rand() % available_notes.size()];
    }

private:
    vector<int> target;
    Scale scale;
    random_device rd;
    mt19937 gen(rd());
};



class Population {
public:
    Population(vector<int> target, float r = 0.1, float m = 0.2, int n_agents = 20, Scale scale = Minor) :
            reproduction_rate(r),
            mutation_rate(m),
            n_agents(n_agents),
            target(target),
            scale(scale),
            n_generations(0)
    {
        vector<int> notes = {};
        notes.push_back(rand() % 7);
//        notes.push_back(scale == Minor ? rand() % 7 : rand() % 12);
        for (int i = 0; i < n_agents; ++i) {
            agents.push_back(Agent(target, scale, notes));
        }
    }

    Agent& operator[](int i) {
        return agents[i];
    }

    int size() {
        return n_agents;
    }

    void process() {
        vector<float> fitnesses;
        for (auto& agent : agents) {
            fitnesses.push_back(agent.evaluate());
        }

        vector<float> selection_weights;
        float fitness_sum = accumulate(fitnesses.begin(), fitnesses.end(), 0.0f);
        for (auto& f : fitnesses) {
            selection_weights.push_back(f / fitness_sum);
        }

        auto best_agent_it = min_element(agents.begin(), agents.end());
        Agent best_agent = *best_agent_it;

        vector<Agent> not_dead;
        discrete_distribution<int> dist(selection_weights.begin(), selection_weights.end());
        for (int i = 0; i < int((1 - reproduction_rate) * n_agents); ++i) {
            not_dead.push_back(agents[dist(mt)]);
        }

        int n_to_reproduce = (n_agents - static_cast<int>(not_dead.size()) * 2);
        vector<Agent> reproduction_pool;
        for (int i = 0; i < n_to_reproduce; ++i) {
            reproduction_pool.push_back(agents[dist(mt)]);
        }

        vector<Agent> new_offspring;
        for (int i = 0; i < n_to_reproduce / 2; ++i) {
            new_offspring.push_back(reproduction_pool[i].reproduce(reproduction_pool[i + n_to_reproduce / 2]));
        }

        agents = not_dead;
        agents.insert(agents.end(), new_offspring.begin(), new_offspring.end());

        // Create indexes from 0 to n_agents
        vector<int> indices(n_agents);
        iota(indices.begin(), indices.end(), 0);
        // Shuffle those indexes
        shuffle(indices.begin(), indices.end(), mt);

        // work out how many mutations to make
        float n_mutations = mutation_rate * static_cast<float>(n_agents);
        for (int i = 0; i < static_cast<int>(n_mutations); ++i) {
            agents[indices[i]].mutate();
        }

        ++n_generations;
    }

private:
    vector<Agent> agents;
    float reproduction_rate;
    float mutation_rate;
    int n_agents;
    vector<int> target;
    vector<int> scale;
    mt19937 mt{random_device{}()};
    int n_generations;
};

#endif //GENOME_EVOLUTION_H
