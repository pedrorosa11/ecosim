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
        // <YOUR CODE HERE>
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
        // Simulate the next iteration
for (uint32_t i = 0; i < NUM_ROWS; ++i) {
    for (uint32_t j = 0; j < NUM_ROWS; ++j) {
        entity_t &current_entity = entity_grid[i][j];

        if (current_entity.type == empty) {
            // Se a célula estiver vazia, continue para a próxima célula
            continue;
        }

        // Implemente o comportamento específico de cada tipo de entidade (plantas, herbívoros, carnívoros)
        switch (current_entity.type) {
            case plant:
                // Verifique a probabilidade de crescimento da planta
                if (random_action(PLANT_REPRODUCTION_PROBABILITY)) {
                    // Encontre uma célula adjacente vazia aleatória
                    pos_t random_empty_cell = find_random_empty_adjacent_cell(i, j);
                    if (random_empty_cell.i != -1 && random_empty_cell.j != -1) {
                        // Crie uma nova planta na célula vazia
                        entity_grid[random_empty_cell.i][random_empty_cell.j] = {plant, 0, 0};
                    }
                }

                // Atualize a expectativa de vida das plantas
                current_entity.age++;
                if (current_entity.age >= PLANT_MAXIMUM_AGE) {
                    // A planta morre e a célula fica vazia
                    entity_grid[i][j] = {empty, 0, 0};
                }
                break;

            case herbivore:
                // Implemente o comportamento de herbívoros nesta iteração
                // 1. Movimento
            if (random_action(HERBIVORE_MOVE_PROBABILITY)) {
                int new_i, new_j;
                do {
                    // Escolha uma direção aleatória para se mover
                    int move_x = random_integer(-1, 1);
                    int move_y = random_integer(-1, 1);
                    new_i = i + move_x;
                    new_j = j + move_y;
                } while (new_i < 0 || new_i >= NUM_ROWS || new_j < 0 || new_j >= NUM_ROWS || entity_grid[new_i][new_j].type != empty);
                
                // Deduza 5 unidades de energia pelo movimento
                entity.energy -= 5;

                // Atualize a posição da entidade no grid
                entity_grid[i][j] = { empty, 0, 0 };
                entity_grid[new_i][new_j] = entity;
            }

            // 2. Alimentação
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    int adjacent_i = i + dx;
                    int adjacent_j = j + dy;

                    if (adjacent_i >= 0 && adjacent_i < NUM_ROWS && adjacent_j >= 0 && adjacent_j < NUM_ROWS) {
                        if (entity_grid[adjacent_i][adjacent_j].type == plant) {
                            // Verifique se há plantas adjacentes para comer
                            if (random_action(HERBIVORE_EAT_PROBABILITY)) {
                                // Coma a planta
                                entity.energy += 30;
                                entity_grid[adjacent_i][adjacent_j] = { empty, 0, 0 };
                            }
                        }
                    }
                }
            }

            // 3. Reprodução
            if (entity.energy > 20 && random_action(HERBIVORE_REPRODUCTION_PROBABILITY)) {
                int new_i, new_j;
                do {
                    // Escolha uma célula adjacente vazia para colocar a prole
                    int move_x = random_integer(-1, 1);
                    int move_y = random_integer(-1, 1);
                    new_i = i + move_x;
                    new_j = j + move_y;
                } while (new_i < 0 || new_i >= NUM_ROWS || new_j < 0 || new_j >= NUM_ROWS || entity_grid[new_i][new_j].type != empty);
                
                // Deduza 10 unidades de energia pelo ato de reprodução
                entity.energy -= 10;

                // Crie a prole
                entity_t offspring = { herbivore, HERBIVORE_INITIAL_ENERGY, HERBIVORE_INITIAL_AGE };
                entity_grid[new_i][new_j] = offspring;
            }

            // 4. Morte
            if (entity.energy <= 0) {
                // A energia chegou a 0, o herbívoro morre
                entity_grid[i][j] = { empty, 0, 0 };
            }
                // Exemplo: Movimento, busca de comida (plantas), reprodução, envelhecimento, etc.
                break;

            case carnivore:
                // Implemente o comportamento de carnívoros nesta iteração

                // 1. Movimento
            if (random_action(CARNIVORE_MOVE_PROBABILITY)) {
                int new_i, new_j;
                do {
                    // Escolha uma direção aleatória para se mover
                    int move_x = random_integer(-1, 1);
                    int move_y = random_integer(-1, 1);
                    new_i = i + move_x;
                    new_j = j + move_y;
                } while (new_i < 0 || new_i >= NUM_ROWS || new_j < 0 || new_j >= NUM_ROWS || entity_grid[new_i][new_j].type != empty);
                
                // Deduza 5 unidades de energia pelo movimento
                entity.energy -= 5;

                // Atualize a posição da entidade no grid
                entity_grid[i][j] = { empty, 0, 0 };
                entity_grid[new_i][new_j] = entity;
            }

            // 2. Alimentação
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    int adjacent_i = i + dx;
                    int adjacent_j = j + dy;

                    if (adjacent_i >= 0 && adjacent_i < NUM_ROWS && adjacent_j >= 0 && adjacent_j < NUM_ROWS) {
                        if (entity_grid[adjacent_i][adjacent_j].type == herbivore) {
                            // Verifique se há herbívoros adjacentes para comer
                            // Carnívoros têm 100% de chance de comer um herbívoro quando adjacente
                            // Ganhe 20 unidades de energia ao comer um herbívoro
                            entity.energy += 20;
                            entity_grid[adjacent_i][adjacent_j] = { empty, 0, 0 };
                        }
                    }
                }
            }

            // 3. Reprodução
            if (entity.energy > 20 && random_action(CARNIVORE_REPRODUCTION_PROBABILITY)) {
                int new_i, new_j;
                do {
                    // Escolha uma célula adjacente vazia para colocar a prole
                    int move_x = random_integer(-1, 1);
                    int move_y = random_integer(-1, 1);
                    new_i = i + move_x;
                    new_j = j + move_y;
                } while (new_i < 0 || new_i >= NUM_ROWS || new_j < 0 || new_j >= NUM_ROWS || entity_grid[new_i][new_j].type != empty);
                
                // Deduza 10 unidades de energia pelo ato de reprodução
                entity.energy -= 10;

                // Crie a prole
                entity_t offspring = { carnivore, CARNIVORE_INITIAL_ENERGY, CARNIVORE_INITIAL_AGE };
                entity_grid[new_i][new_j] = offspring;
            }

            // 4. Morte
            if (entity.energy <= 0) {
                // A energia chegou a 0, o carnívoro morre
                entity_grid[i][j] = { empty, 0, 0 };
            }

                // Exemplo: Movimento, busca de presas (herbívoros), reprodução, envelhecimento, etc.
                break;
        }
    }
}



        
        
        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        return json_grid.dump(); });
    app.port(8080).run();

    return 0;
}