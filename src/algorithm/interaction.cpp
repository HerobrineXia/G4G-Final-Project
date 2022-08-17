float PRECISION = 0.5f; 

int raytracing(glm::vec3& viewPos, glm::vec3& direction, float distance, vector<vector<vector<int>>>* maze, int pos[], int pos2[]){
    if(distance > 0){
        glm::vec3 newPos = glm::vec3(viewPos + direction * PRECISION);
        int row = (int)round(newPos[0])+mazeSize;
        int height = (int)round(newPos[1]);
        int col = (int)round(newPos[2])+mazeSize;
        if(!(row < 0 || row >= mazeSize * 2 + 1 || col < 0 || col >= mazeSize * 2 + 1 || height < 0 || height > mazeHeight)){
            int block = (*maze)[height][row][col];
            if(block != 0){
                pos[0] = height;
                pos[1] = row;
                pos[2] = col;
                pos2[0] = (int)round(viewPos[1]);
                pos2[1] = (int)round(viewPos[0])+mazeSize;
                pos2[2] = (int)round(viewPos[2])+mazeSize;
                return 1;
            }else{
                return raytracing(newPos, direction, distance - PRECISION, maze, pos, pos2);
            }
        }
    }
    return -1;
}