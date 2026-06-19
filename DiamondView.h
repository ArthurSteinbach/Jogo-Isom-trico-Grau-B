#ifndef DiamondView_h
#define DiamondView_h

#include "TilemapView.h"

class DiamondView : public TilemapView {
public:
    void computeDrawPosition(const int col, const int row, const float tw, const float th, float &targetx, float &targety) const {
        // Fórmulas baseadas no slide 20 do material da Unisinos
        targetx = (col * tw / 2.0f) + (row * tw / 2.0f);
        targety = (col * th / 2.0f) - (row * th / 2.0f);
    }
    
    void computeMouseMap(int &col, int &row, const float tw, const float th, const float mx, const float my) const {
        // Função matemática inversa para calcular onde o mouse clicou
        float tw2 = tw / 2.0f;
        float th2 = th / 2.0f;
        
        col = (int)((mx / tw2 + my / th2) / 2.0f);
        row = (int)((mx / tw2 - my / th2) / 2.0f);
    }
    
    void computeTileWalking(int &col, int &row, const int direction) const {
        switch(direction){
            case DIRECTION_NORTH: row++; col++; break;
            case DIRECTION_SOUTH: row--; col--; break;
            case DIRECTION_EAST:  col++; row--; break;
            case DIRECTION_WEST:  col--; row++; break;
        }
    } 
};

#endif /* DiamondView_h */