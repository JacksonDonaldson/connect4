#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <limits>

using namespace std;


enum Dot{
    AI,
    Human,
    Empty
};



class GridSolver{

    //tie == human win
    enum Result{
        aiWin,
        humanWin
    };

    public:
        GridSolver(vector<vector<Dot>> grid) : grid(grid), realMoveCount(0){}


        //returns true if AI has won, otherwise false
        bool playNextMove(int humanColumn){
            playMove(humanColumn, Dot::Human);
            realMoveCount += 2; //human and our move


            vector<pair<int, int>> moves;
            for(int col = 0; col < 6; col++){
                if(grid[0][col] != Dot::Empty){
                    continue;
                }
                int row = playMove(col, Dot::AI);
                int score = getConnectivity(row, col);
                if(score >= 4){
                    return true;
                }

                auto res = memo.find(hash(grid));
                if(res != memo.end()){
                    if(res->second == Result::aiWin){
                        return false;
                    }
                    unplayMove(col);
                    continue;
                }
                unplayMove(col);
                moves.push_back({score, col});
                continue;
            }


            sort(moves.begin(), moves.end(), std::greater<pair<int, int>>());

            for(auto move : moves){
                playMove(move.second, Dot::AI);
                //printGrid();
                if(gridSolve(realMoveCount) == Result::aiWin){
                    return false;
                }
                unplayMove(move.second);
            }

            //it's a solved game; we shouldn't get here
            assert(false);
            return false;
        }

        void printGrid(){
            for(vector<Dot> r : grid){
                cout << "|";
                for(Dot v : r){
                    char res;
                    if(v == Dot::Empty) res = ' ';
                    if(v == Dot::AI) res = 'x';
                    if(v == Dot::Human) res = 'o'; 
                    cout << res << "|";
                }
                cout << endl;
            }
            cout << " ";
            for(int i = 0; i < 7; i++){
                cout << i << " ";
            }
            cout << endl;
        }

    private:
        vector<vector<Dot>> grid;
        int realMoveCount;
        unordered_map<long, Result> memo;
        

        //returns the row of the move played
        int playMove(int col, Dot player){
            for(int i = 5; i >=0; i--){
                if(grid[i][col] == Dot::Empty){
                    grid[i][col] = player;
                    return i;
                }
            }
            //if we get here you called this function wrong
            assert(false);
            return -1;
        }

        //returns the row of the move removed
        int unplayMove(int col){
            for(int i =0; i <6; i++){
                if(grid[i][col] != Dot::Empty){
                    grid[i][col] = Dot::Empty;
                    return i;
                }
            }
            assert(false);
            return -1;
        }

        //plan:
        //-memoize stuff
        //-based on the move maybe?
        //-can do that if we run out of memory, for now let's just do vector<vector<Dot>> -> Result map
        //grid: Human was the last to play, now see if we can win.
        Result gridSolve(int moveCount){
            // if(moveCount < 8){
            //     cout << moveCount << endl;
            // }

            //1. check if this grid has already been solved
            //2. check if the grid is full (movecount = max)
            //3. check each possible move we can make;
            //      is it already solved?
            //      if it is: if win, return win, otherwise, just remove from consideration (in either case we don't need to use it)
            //      if it isn't: maybe get a score for the move (does it create chain? does it set us up well?)
            //4. recurse on each possible move, ordered by score (return early if we can win/ tie)
            //check if we're a human or an AI
            Dot currentPlayer;
            Result desiredResult;
            Result undesiredResult;
            if(moveCount % 2 == 1){
                //cout << "ai" << endl;
                currentPlayer = Dot::AI;
                desiredResult = Result::aiWin;
                undesiredResult = Result::humanWin;
            }
            else{
                //cout << "human" << endl;
                if(moveCount == 42){
                    return Result::humanWin; //tie
                }
                currentPlayer = Dot::Human;
                desiredResult = Result::humanWin;
                undesiredResult = Result::aiWin;
            }
            auto res = memo.find(hash(grid));
            if(res != memo.end()){
                return res->second;
            }

            //grid can't be full (we move first & there's an even space count)
            vector<pair<int, int>> moves;
            for(int col = 0; col < 4; col++){
                //check column i

                //check if already full
                if(grid[0][col] != Dot::Empty){
                    continue;
                }

                //play move
                int row = playMove(col, currentPlayer);

                if(memo.count(hash(grid))){
                    if(memo[hash(grid)] == desiredResult){
                        unplayMove(col);
                        memo[hash(grid)] = desiredResult;
                        printGrid();
                        return desiredResult;
                    }
                    //this move results in a human win 
                    unplayMove(col);
                    continue;
                }

                int score = getConnectivity(row, col);

                if(score >= 4){
                    memo[hash(grid)] = desiredResult;
                    //printGrid();
                    unplayMove(col);
                    memo[hash(grid)] = desiredResult;
                    //printGrid();

                    //cout << endl << endl;
                    return desiredResult;
                }
                
                //we can't rule this move out, so just track it for later use.
                unplayMove(col);
                moves.push_back({score, col});
            }


            //preanalysis done; now evaluate each possible move
            
            //start with the values with the highest score 
            sort(moves.begin(), moves.end(), std::greater<pair<int, int>>());//, [](pair<int, int> p1, pair<int, int> p2) {return p1.first < p2.first;});

            for(auto move : moves){

                playMove(move.second, currentPlayer);
                Result r = gridSolve( moveCount + 1);
                unplayMove(move.second);
                if(r == desiredResult){
                    memo[hash(grid)] = r;
                    return r;
                }
            }
            memo[hash(grid)] = undesiredResult;
            return undesiredResult;
        }

        int getDist(int rowChange, int colChange, int row, int col){
            Dot target = grid[row][col];
            int dist = 1;
            while(row + rowChange * dist >= 0 && row + rowChange * dist < 6 && col + colChange * dist >= 0 && col + colChange * dist <=6 && (grid[row + rowChange * dist][col + colChange * dist] == target)){
                dist++;
            }
            dist--;
            return dist;
        }

        int getConnectivity(int row, int col){
            
            //horizontal
            int maxDist = -1;
            int dist1 = getDist(0, -1, row, col);
            int dist2 = getDist(0, 1, row, col);

            maxDist = max(maxDist, dist1+dist2);

            //vertical
            dist1 = getDist(1, 0, row, col);
            dist2 = getDist(-1, 0, row, col);
            maxDist = max(maxDist, dist1+dist2);

            //diagonal /
            dist1 = getDist(1, 1, row, col);
            dist2 = getDist(-1, -1, row, col);
            maxDist = max(maxDist, dist1+dist2);
            //diagonal '\'
            dist1 = getDist(1, -1, row, col);
            dist2 = getDist(-1, 1, row, col);
            maxDist = max(maxDist, dist1+dist2);

            return maxDist + 1; //+1 for the dot at grid[row][col]
        }

        //will return a unique long for every possible grid, which is why this works
        size_t hash(const vector<vector<Dot>> & grid){
            const int maxSize = 64; //each individual column has at most 64 states

            size_t result;
            for(int c = 0; c < 7; c++){
                int columnValue = 0;
                for(int r = 5; r >=0; r--){
                    if(grid[r][c] == Dot::Empty){
                        break;
                    }

                    columnValue = (columnValue << 1) + (grid[r][c] == Dot::AI) ? 1 : 0;
                }

                result = (result << 6) + columnValue;
            }
            return result; // we only need 7 * 6 = 42 bits
        }
};



int main(){
    vector<vector<Dot>> grid = {{Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty},
    {Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty},
    {Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty},
    {Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty},
    {Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty,Dot::Empty},
    {Dot::Empty,Dot::Empty,Dot::Empty,Dot::AI,Dot::Empty,Dot::Empty,Dot::Empty},
    };
    GridSolver game(grid);
    while(true){
        game.printGrid();
        cout << "Select column (0-6)\n";
        int result = -1;
        while(0 > result || 6 < result){
            cin >> result;
        }

        bool won = game.playNextMove(result);
        //fun optimization: we don't have to check if the player won, because they can't

        cout << "Computing optimal response...\n";

        if(won){
            game.printGrid();
            cout << "you lose...\n";
            break;
        }
    }

}