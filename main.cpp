#include "extras.cpp"

struct simulated_annealing_agent_basic {
	Graph G;
	ld T;
	void init(Graph &G_in) {
		G = G_in;
		T = 1;
	}
	void step() {
		ll node = rand() % G.V;
		ll current_team = G.nodes[node].team;
		ll team = rand() % (sz(G.teams) - 1) + 1;
		ld score = get_score(G);
		G.nodes[node].team = team;
		G.teams[team].nodes.pb(node);
		G.teams[current_team].nodes.erase(find(all(G.teams[current_team].nodes), node));
		ld new_score = get_score(G);
		if (new_score < score) {
			return;
		}
		ld p = exp((score - new_score) / T);
		if (rand() % 1000000 < p * 1000000) {
			return;
		}
		G.nodes[node].team = current_team;
		G.teams[team].nodes.pop_back();
		G.teams[current_team].nodes.pb(node);
	}
};

struct simulated_annealing_agent_weighted {
	Graph G;
	ld T;
	void init(Graph &G_in) {
		G = G_in;
		T = 1;
	}
	void step() {
		vector<ld> node_weights(G.V);
		FOR (i, G.V) {
			ll team = G.nodes[i].team;
			for (ll link : G.nodes[i].links) {
				ll neighbor = G.links[link].source + G.links[link].target - i;
				if (G.nodes[neighbor].team == team) {
					node_weights[i] += G.links[link].weight;
				}
			}
		}
		// FOR (i, G.V) {
		// 	weights[i] = 1 / (weights[i] + 1);
		// }
		ll node = weighted_random(node_weights);
		ll current_team = G.nodes[node].team;
		vector<ld> team_weights(sz(G.teams));
		FOR (i, G.V) {
			ll team = G.nodes[i].team;
			for (ll link : G.nodes[i].links) {
				ll neighbor = G.links[link].source + G.links[link].target - i;
				if (G.nodes[neighbor].team == team) {
					team_weights[team] += G.links[link].weight;
				}
			}
		}
		FOB (i, 1, sz(team_weights)) {
			team_weights[i] = 1 / (team_weights[i] + 1);
		}
		ll team = weighted_random(team_weights, 1);
		ld score = get_score(G);
		G.nodes[node].team = team;
		G.teams[team].nodes.pb(node);
		G.teams[current_team].nodes.erase(find(all(G.teams[current_team].nodes), node));
		ld new_score = get_score(G);
		if (new_score < score) {
			return;
		}
		ld p = exp((score - new_score) / T);
		if (rand() % 1000000 < p * 1000000) {
			return;
		}
		G.nodes[node].team = current_team;
		G.teams[team].nodes.pop_back();
		G.teams[current_team].nodes.pb(node);
	}
};

struct simulated_annealing_agent_timed_weight {
	Graph G;
	ld T;
	ld a, b;
	vector<ld> node_t;
	ld get_weight(ll node) {
		// return 1 / ((node_t[node] / a - b) * (node_t[node] / a - b) + 1);
		return 0.5 * (1 + node_t[node] / a);
	}
	void init(Graph &G_in, ld a = 10000, ld b = 1) {
		G = G_in;
		T = 1;
		this->a = a;
		this->b = b;
		node_t = vector<ld>(G.V);
	}
	void step() {
		vector<ld> node_weights(G.V);
		FOR (i, G.V) {
			node_weights[i] = get_weight(i);
			node_t[i] += 1;
		}
		ll node = weighted_random(node_weights);
		ll current_team = G.nodes[node].team;
		ll team = rand() % (sz(G.teams) - 1) + 1;
		ld score = get_score(G);
		G.nodes[node].team = team;
		G.teams[team].nodes.pb(node);
		G.teams[current_team].nodes.erase(find(all(G.teams[current_team].nodes), node));
		ld new_score = get_score(G);
		if (new_score < score) {
			node_t[node] = 0;
			return;
		}
		ld p = exp((score - new_score) / T);
		if (rand() % 1000000 < p * 1000000) {
			node_t[node] = 0;
			return;
		}
		G.nodes[node].team = current_team;
		G.teams[team].nodes.pop_back();
		G.teams[current_team].nodes.pb(node);
	}
};

Graph simulated_annealing(Graph &G_in) {
	simulated_annealing_agent_basic agent;
	agent.init(G_in);
	while (agent.T > 0.0001) {
		FOR (i, 1000) {
			agent.step();
		}
		agent.T *= 0.99;
	}
	return agent.G;
}

struct genetic_algorithm_controller_basic {
	vector<Graph> population;
	ll generations_left;
	ld mutation_rate;
	void init(Graph &G_in, ll team_count, ll population_size, ll generations, ld mutation_rate = 0.1) {
		generations_left = generations;
		this->mutation_rate = mutation_rate;
		population = vector<Graph>(population_size);
		FOR (i, population_size) {
			population[i] = random_assignment(G_in, team_count);
		}
	}
	void step() {
		vector<Graph> new_population;
		FOR (i, sz(population)) {
			ll a = rand() % sz(population);
			ll b = rand() % sz(population);
			Graph G_a = population[a];
			Graph G_b = population[b];
			Graph G_c = crossover(G_a, G_b);
			G_c = mutate(G_c, mutation_rate);
			new_population.pb(G_c);
		}
		sort(all(new_population), [](Graph &a, Graph &b) {
			return get_score(a) > get_score(b);
		});
		population = vector<Graph>(new_population.begin(), new_population.begin() + sz(population));
		generations_left--;
	}
	Graph crossover(Graph &G_a, Graph &G_b) {
		Graph G_c = G_a;
		FOR (i, G_c.V) {
			if (rand() % 2) {
				G_c.nodes[i].team = G_b.nodes[i].team;
			}
		}
		return G_c;
	}
	Graph mutate(Graph &G, ld rate) {
		Graph G_m = G;
		FOR (i, G_m.V) {
			if (rand() % 1000000 < rate * 1000000) {
				G_m.nodes[i].team = rand() % (sz(G_m.teams) - 1) + 1;
			}
		}
		return G_m;
	}
};

Graph genetic_algorithm(Graph &G_in, ll team_count, ll population_size = 100, ll generations = 1000, ld mutation_rate = 0.1) {
	genetic_algorithm_controller_basic controller;
	controller.init(G_in, team_count, population_size, generations, mutation_rate);
	while (controller.generations_left > 0) {
		controller.step();
	}
	return controller.population[0];
}

int main() {
	// srand(time(NULL));
	set_io("tests/small/random_1/", "sim_anneal");

	Graph G;
	read_input(G);
	read_teams(G, "260494");
	// G = random_assignment(G, 5);
	G = simulated_annealing(G);
	write_output(G);
	return 0;
}