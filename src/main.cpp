#define CROW_MAIN
#define CROW_STATIC_DIR "../public"

#include "crow_all.h"
#include "json.hpp"
#include <random>
#include <thread>
#include <mutex>

static const uint32_t NUM_ROWS = 15;

// Defina as constantes para as probabilidades e valores iniciais
const double PLANT_REPRODUCTION_PROBABILITY = 0.1;
const double HERBIVORE_MOVE_PROBABILITY = 0.3;
const double HERBIVORE_EAT_PROBABILITY = 0.4;
const double HERBIVORE_REPRODUCTION_PROBABILITY = 0.05;
const double CARNIVORE_MOVE_PROBABILITY = 0.3;
const double CARNIVORE_EAT_PROBABILITY = 0.5;
const double CARNIVORE_REPRODUCTION_PROBABILITY = 0.05;
const uint32_t MAXIMUM_ENERGY = 100;
const uint32_t THRESHOLD_ENERGY_FOR_REPRODUCTION = 80;
const uint32_t HERBIVORE_INITIAL_ENERGY = 80;
const uint32_t HERBIVORE_INITIAL_AGE = 0;
const uint32_t CARNIVORE_INITIAL_ENERGY = 100;
const uint32_t CARNIVORE_INITIAL_AGE = 0;

// Defina os tipos de entidades
enum entity_type { empty, plant, herbivore, carnivore };

// Defina uma estrutura de entidade
struct entity_t {
    entity_type type;
    uint32_t energy;
    uint32_t age;
};

// Defina uma estrutura para representar uma posição
struct pos_t {
    int i;
    int j;
};

// Defina um gerador de números aleatórios
std::mt19937 gen(std::random_device{}());

// Defina a grade de entidades
std::vector<std::vector<entity_t>> entity_grid(NUM_ROWS, std::vector<entity_t>(NUM_ROWS, { empty, 0, 0 }));

// Mutexes para exclusão mútua
std::mutex grid_mutex;

// Função para gerar um número inteiro aleatório entre min e max
int random_integer(int min, int max) {
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(gen);
}

// Função para gerar um número de ponto flutuante aleatório entre 0 e 1
double random_action(double probability) {
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(gen) < probability;
}

// Função para converter a grade de entidades em um objeto JSON
nlohmann::json entityGridToJson(const std::vector<std::vector<entity_t>>& grid) {
    nlohmann::json json_grid;
    for (const auto& row : grid) {
        nlohmann::json json_row;
        for (const auto& entity : row) {
            nlohmann::json json_entity = {
                { "type", entity.type },
                { "energy", entity.energy },
                { "age", entity.age }
            };
            json_row.push_back(json_entity);
        }
        json_grid.push_back(json_row);
    }
    return json_grid;
}

int main() {
    crow::SimpleApp app;

    // Endpoint para iniciar a simulação
    CROW_ROUTE(app, "/start-simulation").methods("POST"_method)([](crow::request &req, crow::response &res) {
        // Analisar o corpo da solicitação JSON
        nlohmann::json request_body = nlohmann::json::parse(req.body);

        // Validar a solicitação
        uint32_t total_entities = (uint32_t)request_body["plants"] + (uint32_t)request_body["herbivores"] + (uint32_t)request_body["carnivores"];
        if (total_entities > NUM_ROWS * NUM_ROWS) {
            res.code = 400;
            res.body = "Muitas entidades";
            res.end();
            return;
        }

        // Limpar a grade de entidades
        std::lock_guard<std::mutex> lock(grid_mutex); // Bloquear o mutex durante a atualização
        entity_grid.clear();
        entity_grid.assign(NUM_ROWS, std::vector<entity_t>(NUM_ROWS, { empty, 0, 0 }));

        // Criar as entidades (plantas, herbívoros e carnívoros) com base na solicitação
        uint32_t num_plants = (uint32_t)request_body["plants"];
        uint32_t num_herbivores = (uint32_t)request_body["herbivores"];
        uint32_t num_carnivores = (uint32_t)request_body["carnivores"];

        for (uint32_t i = 0; i < num_plants; ++i) {
            int row, col;
            do {
                row = random_integer(0, NUM_ROWS - 1);
                col = random_integer(0, NUM_ROWS - 1);
            } while (entity_grid[row][col].type != empty);

            entity_grid[row][col] = { plant, MAXIMUM_ENERGY, 0 };
        }

        for (uint32_t i = 0; i < num_herbivores; ++i) {
            int row, col;
            do {
                row = random_integer(0, NUM_ROWS - 1);
                col = random_integer(0, NUM_ROWS - 1);
            } while (entity_grid[row][col].type != empty);

            entity_grid[row][col] = { herbivore, MAXIMUM_ENERGY, 0 };
        }

        for (uint32_t i = 0; i < num_carnivores; ++i) {
            int row, col;
            do {
                row = random_integer(0, NUM_ROWS - 1);
                col = random_integer(0, NUM_ROWS - 1);
            } while (entity_grid[row][col].type != empty);

            entity_grid[row][col] = { carnivore, MAXIMUM_ENERGY, 0 };
        }

        // Retornar a representação JSON da grade de entidades
        nlohmann::json json_grid = entityGridToJson(entity_grid);
        res.body = json_grid.dump();
        res.end();

    });

    // Endpoint para a próxima iteração da simulação
    CROW_ROUTE(app, "/next-iteration").methods("GET"_method)([]() {
        // Iniciar a simulação em um loop (por exemplo, 100 iterações)
        for (int iteration = 0; iteration < 100; ++iteration) {
            // Simular a próxima iteração
            std::lock_guard<std::mutex> lock(grid_mutex); // Bloquear o mutex durante a simulação
            std::vector<std::vector<entity_t>> new_entity_grid = entity_grid;

            for (uint32_t i = 0; i < NUM_ROWS; ++i) {
                for (uint32_t j = 0; j < NUM_ROWS; ++j) {
                    entity_t &current_entity = entity_grid[i][j];
                    entity_t &new_entity = new_entity_grid[i][j];

                    // Implementar a lógica de comportamento apropriada para cada tipo de entidade
                    switch (current_entity.type) {
                        case empty:
                            // Célula vazia, nenhuma ação necessária
                            break;
                        case plant:
                            // Lógica para plantas (por exemplo, crescimento, reprodução)
                            if (random_action(PLANT_REPRODUCTION_PROBABILITY)) {
                                std::vector<pos_t> empty_adjacent_cells;
                                if (i > 0 && entity_grid[i - 1][j].type == empty) {
                                    empty_adjacent_cells.push_back({ i - 1, j });
                                }
                                if (i < NUM_ROWS - 1 && entity_grid[i + 1][j].type == empty) {
                                    empty_adjacent_cells.push_back({ i + 1, j });
                                }
                                if (j > 0 && entity_grid[i][j - 1].type == empty) {
                                    empty_adjacent_cells.push_back({ i, j - 1 });
                                }
                                if (j < NUM_ROWS - 1 && entity_grid[i][j + 1].type == empty) {
                                    empty_adjacent_cells.push_back({ i, j + 1 });
                                }

                                if (!empty_adjacent_cells.empty()) {
                                    std::uniform_int_distribution<size_t> rand_empty_cell(0, empty_adjacent_cells.size() - 1);
                                    size_t chosen_index = rand_empty_cell(gen);
                                    pos_t new_plant_pos = empty_adjacent_cells[chosen_index];
                                    new_entity_grid[new_plant_pos.i][new_plant_pos.j].type = plant;
                                }
                            }
                            break;
                        case herbivore:
                            // Lógica para herbívoros (por exemplo, movimento, alimentação, reprodução)
                            if (random_action(HERBIVORE_MOVE_PROBABILITY)) {
                                // Herbívoro se move
                                int current_i = static_cast<int>(i);
                                int current_j = static_cast<int>(j);
                                std::vector<pos_t> possible_moves;

                                if (current_i > 0 && entity_grid[current_i - 1][current_j].type == empty) {
                                    possible_moves.push_back({current_i - 1, current_j}); // Mover para cima
                                }
                                if (current_i < NUM_ROWS - 1 && entity_grid[current_i + 1][current_j].type == empty) {
                                    possible_moves.push_back({current_i + 1, current_j}); // Mover para baixo
                                }
                                if (current_j > 0 && entity_grid[current_i][current_j - 1].type == empty) {
                                    possible_moves.push_back({current_i, current_j - 1}); // Mover para a esquerda
                                }
                                if (current_j < NUM_ROWS - 1 && entity_grid[current_i][current_j + 1].type == empty) {
                                    possible_moves.push_back({current_i, current_j + 1}); // Mover para a direita
                                }

                                if (!possible_moves.empty()) {
                                    int random_index = random_integer(0, possible_moves.size() - 1);
                                    int new_i = possible_moves[random_index].i;
                                    int new_j = possible_moves[random_index].j;

                                    new_entity_grid[new_i][new_j] = current_entity;
                                    new_entity_grid[current_i][current_j].type = empty;
                                }
                            }

                            if (random_action(HERBIVORE_EAT_PROBABILITY)) {
                                // Herbívoro tenta comer uma planta
                               int current_i = static_cast<int>(i);
                                int current_j = static_cast<int>(j);
                                for (int direction = 0; direction < 4; ++direction) {
                                    int new_i = current_i;
                                    int new_j = current_j;

                                    switch (direction) {
                                        case 0:
                                            if (current_i > 0) {
                                                new_i = current_i - 1;
                                            }
                                            break;
                                        case 1:
                                            if (current_i < NUM_ROWS - 1) {
                                                new_i = current_i + 1;
                                            }
                                            break;
                                        case 2:
                                            if (current_j > 0) {
                                                new_j = current_j - 1;
                                            }
                                            break;
                                        case 3:
                                            if (current_j < NUM_ROWS - 1) {
                                                new_j = current_j + 1;
                                            }
                                            break;
                                    }

                                    if (entity_grid[new_i][new_j].type == plant) {
                                        entity_grid[new_i][new_j].type = empty;
                                        current_entity.energy += 30;
                                    }
                                }
                            }

                            if (random_action(HERBIVORE_REPRODUCTION_PROBABILITY)) {
                                // Herbívoro tenta se reproduzir
                                if (current_entity.energy > THRESHOLD_ENERGY_FOR_REPRODUCTION) {
                                    for (int direction = 0; direction < 4; ++direction) {
                                        int new_i = i;
                                        int new_j = j;

                                        switch (direction) {
                                            case 0:
                                                if (i > 0) {
                                                    new_i = i - 1;
                                                }
                                                break;
                                            case 1:
                                                if (i < NUM_ROWS - 1) {
                                                    new_i = i + 1;
                                                }
                                                break;
                                            case 2:
                                                if (j > 0) {
                                                    new_j = j - 1;
                                                }
                                                break;
                                            case 3:
                                                if (j < NUM_ROWS - 1) {
                                                    new_j = j + 1;
                                                }
                                                break;
                                        }

                                        if (entity_grid[new_i][new_j].type == empty) {
                                            entity_grid[new_i][new_j].type = herbivore;
                                            entity_grid[new_i][new_j].energy = HERBIVORE_INITIAL_ENERGY;
                                            entity_grid[new_i][new_j].age = HERBIVORE_INITIAL_AGE;
                                            current_entity.energy -= 10;
                                            break;
                                        }
                                    }
                                }
                            }
                            break;
                        case carnivore:
                            // Lógica para carnívoros (por exemplo, movimento, alimentação, reprodução)
                            if (random_action(CARNIVORE_MOVE_PROBABILITY)) {
                                int random_direction = random_integer(0, 3);
                                int new_i = i;
                                int new_j = j;

                                switch (random_direction) {
                                    case 0:
                                        if (i > 0) {
                                            new_i = i - 1;
                                        }
                                        break;
                                    case 1:
                                        if (i < NUM_ROWS - 1) {
                                            new_i = i + 1;
                                        }
                                        break;
                                    case 2:
                                        if (j > 0) {
                                            new_j = j - 1;
                                        }
                                        break;
                                    case 3:
                                        if (j < NUM_ROWS - 1) {
                                            new_j = j + 1;
                                        }
                                        break;
                                }

                                if (entity_grid[new_i][new_j].type == herbivore) {
                                    entity_grid[new_i][new_j] = current_entity;
                                    entity_grid[i][j].type = empty;
                                }
                            }

                            if (random_action(CARNIVORE_EAT_PROBABILITY)) {
                                int random_direction = random_integer(0, 3);
                                int new_i = i;
                                int new_j = j;

                                switch (random_direction) {
                                    case 0:
                                        if (i > 0) {
                                            new_i = i - 1;
                                        }
                                        break;
                                    case 1:
                                        if (i < NUM_ROWS - 1) {
                                            new_i = i + 1;
                                        }
                                        break;
                                    case 2:
                                        if (j > 0) {
                                            new_j = j - 1;
                                        }
                                        break;
                                    case 3:
                                        if (j < NUM_ROWS - 1) {
                                            new_j = j + 1;
                                        }
                                        break;
                                }

                                if (entity_grid[new_i][new_j].type == herbivore) {
                                    entity_grid[new_i][new_j].type = empty;
                                    current_entity.energy += 50;
                                }
                            }

                            if (random_action(CARNIVORE_REPRODUCTION_PROBABILITY)) {
                                if (current_entity.energy > THRESHOLD_ENERGY_FOR_REPRODUCTION) {
                                    for (int direction = 0; direction < 4; ++direction) {
                                        int new_i = i;
                                        int new_j = j;

                                        switch (direction) {
                                            case 0:
                                                if (i > 0) {
                                                    new_i = i - 1;
                                                }
                                                break;
                                            case 1:
                                                if (i < NUM_ROWS - 1) {
                                                    new_i = i + 1;
                                                }
                                                break;
                                            case 2:
                                                if (j > 0) {
                                                    new_j = j - 1;
                                                }
                                                break;
                                            case 3:
                                                if (j < NUM_ROWS - 1) {
                                                    new_j = j + 1;
                                                }
                                                break;
                                        }

                                        if (entity_grid[new_i][new_j].type == empty) {
                                            entity_grid[new_i][new_j].type = carnivore;
                                            entity_grid[new_i][new_j].energy = CARNIVORE_INITIAL_ENERGY;
                                            entity_grid[new_i][new_j].age = CARNIVORE_INITIAL_AGE;
                                            current_entity.energy -= 20;
                                            break;
                                        }
                                    }
                                }
                            }
                            break;
                    }

                    // Atualizar a idade e energia da entidade
                    current_entity.age++;
                    current_entity.energy--;

                    if (current_entity.energy <= 0) {
                        // A entidade morre se sua energia for esgotada
                        new_entity_grid[i][j] = { empty, 0, 0 };
                    }
                }
            }

            // Atualizar a grade de entidades com a cópia temporária
            entity_grid = new_entity_grid;
        }

        // Retorne a representação JSON da grade de entidades
        nlohmann::json json_grid = entityGridToJson(entity_grid);
        return json_grid.dump();
    });

    app.port(8080).run(); // Use port 8081 instead of 8080

    return 0;
}
