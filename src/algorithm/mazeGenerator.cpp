#include <vector>
#include <iostream>

using namespace std;


int randTexture(int textureCount){
    return rand() % textureCount + 1;
}

int getVisitedCell(int row, int col, vector<vector<int>>* maze){
    int visit = 0;
    if(row - 1 > 0 && (*maze)[row - 1][col] == 0){
        ++visit;
    }
    if(col - 1 > 0 && (*maze)[row][col - 1] == 0){
        ++visit;
    }
    if(row + 1 < (*maze).size() - 1 && (*maze)[row + 1][col] == 0){
        ++visit;
    }
    if(col + 1 < (*maze)[0].size() - 1 && (*maze)[row][col + 1] == 0){
        ++visit;
    }
    return visit;
}

void addNeighborWall(int row, int col, vector<vector<int>>* maze, vector<pair<int,int>>* wall){
    if(row - 1 > 0 && (*maze)[row - 1][col] != 0){
        pair<int,int> pos (row - 1, col);
        wall->push_back(pos);
    }
    if(col - 1 > 0 && (*maze)[row][col - 1] != 0){
        pair<int,int> pos (row, col - 1);
        wall->push_back(pos);
    }
    if(row + 1 < (*maze).size() - 1 && (*maze)[row + 1][col] != 0){
        pair<int,int> pos (row + 1, col);
        wall->push_back(pos);
    }
    if(col + 1 < (*maze)[0].size() - 1 && (*maze)[row][col + 1] != 0){
        pair<int,int> pos (row, col + 1);
        wall->push_back(pos);
    }
}

pair<int, int> addRandomCell(int row, int col, vector<vector<int>>* maze){
    vector<pair<int, int>> unvisit;
    if(row % 2 == 0 && (*maze)[row - 1][col] != 0){
        pair<int, int> pos (row-1, col);
        unvisit.push_back(pos);
    }
    if(row % 2 == 0 && (*maze)[row + 1][col] != 0){
        pair<int, int> pos (row+1, col);
        unvisit.push_back(pos);
    }
    if(col % 2 == 0 && (*maze)[row][col - 1] != 0){
        pair<int, int> pos (row, col-1);
        unvisit.push_back(pos);
    }
    if(col % 2 == 0 && (*maze)[row][col + 1] != 0){
        pair<int, int> pos (row, col+1);
        unvisit.push_back(pos);
    }
    if(unvisit.size() == 0){
        pair<int, int> pos (-1, -1);
        return pos;
    }
    int index = rand() % unvisit.size();
    return unvisit[index];

}

vector<vector<vector<int>>> generateMaze(int row, int col, int height, int textureCount){
    vector<vector<int>> maze;
    vector<pair<int,int>> wall;
    for(int i = 0; i < row; ++i){
        vector<int> rows;
        for(int j = 0; j < col; ++j){
            rows.push_back(randTexture(textureCount));
        }
        maze.push_back(rows);
    }

    // Middle
    maze[row / 2][col / 2] = 0;
    addNeighborWall(row / 2, col / 2, &maze, &wall);

    while(wall.size() != 0){
        int index = rand() % wall.size();
        pair<int, int> pos = wall[index];
        if(getVisitedCell(pos.first, pos.second, &maze) == 1){
            pair<int, int> newPos = addRandomCell(pos.first, pos.second, &maze);
            if(newPos.first != -1){
                maze[pos.first][pos.second] = 0;
                maze[newPos.first][newPos.second] = 0;
                addNeighborWall(newPos.first, newPos.second, &maze, &wall);
            }
        }
        wall.erase(wall.begin() + index);
    }
    // Exit
    maze[0][0] = 0;
    maze[0][1] = 0;
    maze[1][0] = 0;
    maze[0][col-1] = 0;
    maze[0][col-2] = 0;
    maze[1][col-1] = 0;
    maze[row-1][0] = 0;
    maze[row-2][0] = 0;
    maze[row-1][1] = 0;
    maze[row-1][col-1] = 0;
    maze[row-2][col-1] = 0;
    maze[row-1][col-2] = 0;

    vector<vector<vector<int>>> mapOut;
    for(int i = 0; i <= height; ++i){
        vector<vector<int>> surface;
        for(int j = 0; j < row; ++j){
            vector<int> rows;
            for(int k = 0; k < col; ++k){
                if(i == 0){
                    rows.push_back(1);
                }else{
                    rows.push_back(maze[j][k]);
                }
            }
            surface.push_back(rows);
        }
        mapOut.push_back(surface);
    }

    return mapOut;
}

void printMaze(vector<vector<int>>* maze){
    for(vector<int> row : *maze){
        for(int col: row){
            if(col == 0){
                printf("□");
            }else{
                printf("■");
            }
        }
        printf("\n");
    }
}