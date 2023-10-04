#define CROW_MAIN
#define CROW_STATIC_DIR "../public"

#include "crow_all.h"
#include "json.hpp"
#include <random>

static const uint32_t NUM_ROWS = 15;

// Constants
const uint32_t PLANT_MAXIMUM_AGE = 10;
const uint32_t PLANT_INITIAL_AGE = 0;

const uint32_t HERBIVORE_MAXIMUM_AGE = 50;
const uint32_t HERBIVORE_INITIAL_ENERGY = 100;
const uint32_t HERBIVORE_INITIAL_AGE = 0;

const uint32_t CARNIVORE_MAXIMUM_AGE = 80;
const uint32_t CARNIVORE_INITIAL_ENERGY = 100;
const uint32_t CARNIVORE_INITIAL_AGE = 0;

const uint32_t MAXIMUM_ENERGY = 200;
const uint32_t THRESHOLD_ENERGY_FOR_REPRODUCTION = 20;

// Probabilities
const double PLANT_REPRODUCTION_PROBABILITY = 0.2;
const double HERBIVORE_REPRODUCTION_PROBABILITY = 0.075;
const double CARNIVORE_REPRODUCTION_PROBABILITY = 0.025;
const double HERBIVORE_MOVE_PROBABILITY = 0.7;
const double HERBIVORE_EAT_PROBABILITY = 0.9;
const double CARNIVORE_MOVE_PROBABILITY = 0.5;
const double CARNIVORE_EAT_PROBABILITY = 1.0;

// Type definitions
enum entity_type_t
{
    empty,
    plant,
    herbivore,
    carnivore
};

struct pos_t
{
    uint32_t i;
    uint32_t j;
};

struct entity_t
{
    entity_type_t type;
    int32_t energy;
    int32_t age;
};

// Auxiliary code to convert the entity_type_t enum to a string
NLOHMANN_JSON_SERIALIZE_ENUM(entity_type_t, {
                                                {empty, " "},
                                                {plant, "P"},
                                                {herbivore, "H"},
                                                {carnivore, "C"},
                                            })

// Auxiliary code to convert the entity_t struct to a JSON object
namespace nlohmann
{
    void to_json(nlohmann::json &j, const entity_t &e)
    {
        j = nlohmann::json{{"type", e.type}, {"energy", e.energy}, {"age", e.age}};
    }
}
// Function to generate a random action based on probability
bool random_action(float probability) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    return dis(gen) < probability;
}

// Function to generate a random initial position based on probability
bool random_integer(const int min, const int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

// Simulate random actions for different entities
void simulate_random_actions() {
    // Probabilities for different actions
    float plant_growth_probability = 0.20; // 20% chance of growth
    float herbivore_move_probability = 0.70; // 70% chance to action
    float carnivore_move_probability = 0.60; // 60% chance to action

    // Simulate plant growth
    if (random_action(plant_growth_probability)) {
        std::cout << "Plant grows.\n";
    } else {
        std::cout << "Plant does not grow.\n";
    }

    // Simulate herbivore action
    if (random_action(herbivore_move_probability)) {
        std::cout << "Herbivore moves.\n";
    } else {
        std::cout << "Herbivore does not move.\n";
    }

    // Simulate carnivore action
    if (random_action(carnivore_move_probability)) {
        std::cout << "Carnivore moves.\n";
    } else {
        std::cout << "Carnivore does not move.\n";
    }
}
// Grid that contains the entities
static std::vector<std::vector<entity_t>> entity_grid;


        // Função para criar as plantas aleatoriamente no grid
        void createPlants(int numPlants) {
            for (int i = 0; i < numPlants; ++i) {
                int row, col;
                // Gere posições aleatórias até encontrar uma célula vazia (empty)
                do {
                    row = random_integer(0, NUM_ROWS - 1);
                    col = random_integer(0, NUM_ROWS - 1);
                } while (entity_grid[row][col].type != empty);
                
                // Defina a célula como uma planta
                entity_grid[row][col].type = plant;
            }
        }

        // Função para criar herbivoros aleatoriamente no grid
        void createHerbivores(int numHerbivores) {
            for (int i = 0; i < numHerbivores; ++i) {
                int row, col;
                // Gere posições aleatórias até encontrar uma célula vazia (empty)
                do {
                    row = random_integer(0, NUM_ROWS - 1);
                    col = random_integer(0, NUM_ROWS - 1);
                } while (entity_grid[row][col].type != empty);
                
                // Defina a célula como uma planta
                entity_grid[row][col].type = herbivore;
                entity_grid[row][col].energy = HERBIVORE_INITIAL_ENERGY;
                entity_grid[row][col].age = HERBIVORE_INITIAL_AGE;
            }
        }

        // Função para criar carnivoros aleatoriamente no grid
        void createCarnivores(int numCarnivores) {
            for (int i = 0; i < numCarnivores; ++i) {
                int row, col;
                // Gere posições aleatórias até encontrar uma célula vazia (empty)
                do {
                    row = random_integer(0, NUM_ROWS - 1);
                    col = random_integer(0, NUM_ROWS - 1);
                } while (entity_grid[row][col].type != empty);
                
                // Defina a célula como uma planta
                entity_grid[row][col].type = carnivore;
                entity_grid[row][col].energy = CARNIVORE_INITIAL_ENERGY;
                entity_grid[row][col].age = CARNIVORE_INITIAL_AGE;
            }
        }


int main()
{
    crow::SimpleApp app;

    // Endpoint to serve the HTML page
    CROW_ROUTE(app, "/")
    ([](crow::request &, crow::response &res)
     {
        // Return the HTML content here
        res.set_static_file_info_unsafe("../public/index.html");
        res.end(); });

    CROW_ROUTE(app, "/start-simulation")
        .methods("POST"_method)([](crow::request &req, crow::response &res)
                                { 
        // Parse the JSON request body
        nlohmann::json request_body = nlohmann::json::parse(req.body);

       // Validate the request body 
        uint32_t total_entinties = (uint32_t)request_body["plants"] + (uint32_t)request_body["herbivores"] + (uint32_t)request_body["carnivores"];
        if (total_entinties > NUM_ROWS * NUM_ROWS) {
        res.code = 400;
        res.body = "Too many entities";
        res.end();
        return;
        }

        // Clear the entity grid
        entity_grid.clear();
        entity_grid.assign(NUM_ROWS, std::vector<entity_t>(NUM_ROWS, { empty, 0, 0}));
        
        // Create the entities
        // <YOUR CODE HERE> - Done
        // Código para criar entidades com base nos números fornecidos na solicitação POST
        uint32_t numPlants = (uint32_t)request_body["plants"];
        uint32_t numHerbivores = (uint32_t)request_body["herbivores"];
        uint32_t numCarnivores = (uint32_t)request_body["carnivores"];

        // Chame as funções para criar as entidades com base nos valores fornecidos
        createPlants(numPlants);
        createHerbivores(numHerbivores);
        createCarnivores(numCarnivores);


        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        res.body = json_grid.dump();
        res.end(); });

    // Endpoint to process HTTP GET requests for the next simulation iteration
    CROW_ROUTE(app, "/next-iteration")
        .methods("GET"_method)([]()
                               {
        // Simulate the next iteration
        // Iterate over the entity grid and simulate the behaviour of each entity
        // <YOUR CODE HERE>
void simulateNextIteration() {
    // Create a temporary copy of the entity grid to avoid changes during iteration
    std::vector<std::vector<entity_t>> new_entity_grid = entity_grid;

    for (uint32_t i = 0; i < NUM_ROWS; ++i) {
        for (uint32_t j = 0; j < NUM_ROWS; ++j) {
            entity_t& current_entity = entity_grid[i][j];
            entity_t& new_entity = new_entity_grid[i][j];

            // Verifique o tipo da entidade atual e implemente a lógica de comportamento apropriada
            switch (current_entity.type) {
                case empty:
                    // empty cell, no action needed
                    break;
                case plant:
                    // Logic for plants (e.g., growth, reproduction)
                    if (random_action(PLANT_REPRODUCTION_PROBABILITY)) {
                        // Find an adjacent empty cell for reproduction
                        std::vector<pos_t> empty_adjacent_cells;

                        // Check the cell above
                        if (i > 0 && entity_grid[i - 1][j].type == empty) {
                            empty_adjacent_cells.push_back({i - 1, j});
                        }
                        // Check the cell below
                        if (i < NUM_ROWS - 1 && entity_grid[i + 1][j].type == empty) {
                            empty_adjacent_cells.push_back({i + 1, j});
                        }
                        // Check the cell to the left
                        if (j > 0 && entity_grid[i][j - 1].type == empty) {
                            empty_adjacent_cells.push_back({i, j - 1});
                        }
                        // Check the cell to the right
                        if (j < NUM_ROWS - 1 && entity_grid[i][j + 1].type == empty) {
                            empty_adjacent_cells.push_back({i, j + 1});
                        }

                        // If there are empty adjacent cells, create a new plant in one of them
                        if (!empty_adjacent_cells.empty()) {
                            // Choose a random empty adjacent cell
                            std::uniform_int_distribution<size_t> rand_empty_cell(0, empty_adjacent_cells.size() - 1);
                            size_t chosen_index = rand_empty_cell(gen);
                            pos_t new_plant_pos = empty_adjacent_cells[chosen_index];

                            // Create a new plant in the chosen cell
                            new_entity_grid[new_plant_pos.i][new_plant_pos.j].type = plant;
                        }
                    }

                    break;
                case herbivore:
                    // Logic for herbivores (e.g., movement, eating, reproduction)
                    if (random_action(HERBIVORE_MOVE_PROBABILITY)) {
                        // Herbívoro se move
                        // Implemente a lógica de movimento aqui

                        // Posição atual do herbívoro
                        int current_i = i;
                        int current_j = j;

                        // Lista de possíveis movimentos (células vizinhas)
                        std::vector<pos_t> possible_moves;

                        // Verifique as células vizinhas para determinar onde o herbívoro pode se mover
                        // Certifique-se de verificar limites para não sair do grid
                        if (current_i > 0 && entity_grid[current_i - 1][current_j].type == empty) {
                            possible_moves.push_back({current_i - 1, current_j}); // Movimento para cima
                        }
                        if (current_i < NUM_ROWS - 1 && entity_grid[current_i + 1][current_j].type == empty) {
                            possible_moves.push_back({current_i + 1, current_j}); // Movimento para baixo
                        }
                        if (current_j > 0 && entity_grid[current_i][current_j - 1].type == empty) {
                            possible_moves.push_back({current_i, current_j - 1}); // Movimento para a esquerda
                        }
                        if (current_j < NUM_ROWS - 1 && entity_grid[current_i][current_j + 1].type == empty) {
                            possible_moves.push_back({current_i, current_j + 1}); // Movimento para a direita
                        }

                        // Se houver células disponíveis para mover, escolha aleatoriamente uma delas
                        if (!possible_moves.empty()) {
                            int random_index = random_integer(0, possible_moves.size() - 1);
                            int new_i = possible_moves[random_index].i;
                            int new_j = possible_moves[random_index].j;

                            // Movimente o herbívoro para a nova posição (new_i, new_j)
                            entity_grid[new_i][new_j] = entity_grid[current_i][current_j];
                            entity_grid[current_i][current_j] = {empty, 0, 0};
                        }
                    }
                    if (random_action(HERBIVORE_EAT_PROBABILITY)) {
                        // Herbívoro tenta comer uma planta
                        // Implemente a lógica de alimentação aqui
                        
                        // Posição atual do herbívoro
                        int current_i = i;
                        int current_j = j;

                        // Verifique se há uma planta adjacente (célula com "P")
                        if (current_i > 0 && entity_grid[current_i - 1][current_j].type == plant) {
                            // A planta está acima do herbívoro
                            // Realize a alimentação, ganhe energia e remova a planta
                            entity_grid[current_i - 1][current_j].type = empty; // Remova a planta
                            entity_grid[current_i][current_j].energy += 30; // Ganhe 30 unidades de energia
                        }
                        else if (current_i < NUM_ROWS - 1 && entity_grid[current_i + 1][current_j].type == plant) {
                            // A planta está abaixo do herbívoro
                            // Realize a alimentação, ganhe energia e remova a planta
                            entity_grid[current_i + 1][current_j].type = empty; // Remova a planta
                            entity_grid[current_i][current_j].energy += 30; // Ganhe 30 unidades de energia
                        }
                        else if (current_j > 0 && entity_grid[current_i][current_j - 1].type == plant) {
                            // A planta está à esquerda do herbívoro
                            // Realize a alimentação, ganhe energia e remova a planta
                            entity_grid[current_i][current_j - 1].type = empty; // Remova a planta
                            entity_grid[current_i][current_j].energy += 30; // Ganhe 30 unidades de energia
                        }
                        else if (current_j < NUM_ROWS - 1 && entity_grid[current_i][current_j + 1].type == plant) {
                            // A planta está à direita do herbívoro
                            // Realize a alimentação, ganhe energia e remova a planta
                            entity_grid[current_i][current_j + 1].type = empty; // Remova a planta
                            entity_grid[current_i][current_j].energy += 30; // Ganhe 30 unidades de energia
                        }
                    }

                    if (random_action(HERBIVORE_REPRODUCTION_PROBABILITY)) {
                        // Herbívoro tenta se reproduzir
                        // Implemente a lógica de reprodução aqui

                        // Verifique se a energia do herbívoro é maior que 20 (requisito para reprodução)
                        if (entity_grid[i][j].energy > 20) {
                            // Encontre uma célula vazia adjacente para colocar a prole
                            int empty_i = -1;
                            int empty_j = -1;

                            // Tente encontrar uma célula vazia adjacente aleatoriamente
                            while (empty_i == -1 || empty_j == -1) {
                                int random_direction = random_integer(0, 3);

                                if (random_direction == 0 && i > 0 && entity_grid[i - 1][j].type == empty) {
                                    empty_i = i - 1;
                                    empty_j = j;
                                }
                                else if (random_direction == 1 && i < NUM_ROWS - 1 && entity_grid[i + 1][j].type == empty) {
                                    empty_i = i + 1;
                                    empty_j = j;
                                }
                                else if (random_direction == 2 && j > 0 && entity_grid[i][j - 1].type == empty) {
                                    empty_i = i;
                                    empty_j = j - 1;
                                }
                                else if (random_direction == 3 && j < NUM_ROWS - 1 && entity_grid[i][j + 1].type == empty) {
                                    empty_i = i;
                                    empty_j = j + 1;
                                }
                            }

                            // Crie uma nova prole herbívora na célula vazia encontrada
                            entity_grid[empty_i][empty_j].type = herbivore;
                            entity_grid[empty_i][empty_j].energy = HERBIVORE_INITIAL_ENERGY;
                            entity_grid[empty_i][empty_j].age = HERBIVORE_INITIAL_AGE;

                            // Reduza a energia do herbívoro pai devido à reprodução
                            entity_grid[i][j].energy -= 10;
                        }
                    }

                    break;

                case carnivore:
                    // Lógica para carnívoros (por exemplo, movimento, comer, reprodução)
                    if (random_action(CARNIVORE_MOVE_PROBABILITY)) {
                        // Carnívoro se move
                        // Implemente a lóagica de movimento aqui
                    }
                    if (random_action(CARNIVORE_EAT_PROBABILITY)) {
                        // Carnívoro tenta comer
                        // Implemente a lógica de alimentação aqui
                    }
                    if (random_action(CARNIVORE_REPRODUCTION_PROBABILITY)) {
                        // Carnívoro se reproduz
                        // Implemente a lógica de reprodução aqui
                    }
                    break;
            }
        }
    }

    // Atualize a grade de entidades com a cópia temporária
    entity_grid = new_entity_grid;
}


    
        
        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        return json_grid.dump(); });
    app.port(8080).run();

    return 0;
}