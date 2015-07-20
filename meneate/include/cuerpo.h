#ifndef CUERPO_H
#define CUERPO_H
#include "nodo.h"
#include <vector>
#include <math.h>

class cuerpo
{
    private:
        nodo *pie_d,
             *pie_i,
             *rod_d,
             *rod_i,
             *cad_d,
             *cad_i,
             *mano_d,
             *mano_i,
             *codo_d,
             *codo_i,
             *hom_d,
             *hom_i,
             *cab;
        double avg_x,
               avg_y,
               ds,
               pie_rod,
               rod_cad_d,
               rod_cad_i,
               cad_cad,
               cad_hom_d,
               cad_hom_i,
               hom_hom,
               hom_codo_d,
               hom_codo_i,
               codo_mano,
               cab_homb;
        vector <nodo*> ptr_marcas;
        //<pie_d, pie_i, rod_d, rod_i, cad_d, cad_i, hom_d, hom_i, cab, codo_d, codo_i,mano_d, mano_i>
        vector <nodo> marcas;
        //vector <bool> actualizaciones;
        bool calibrado;
        bool dato;

    public:

        cuerpo();
        void calibrar(vector <nodo> &_marcas);
        void ord_ptr_marcas();

        void identificar_marcas();
        void detect_pies();
        void detect_rodillas();
        void detect_caderas();
        void detect_hombros();
        void detect_cab();
        void detect_codos();
        void detect_manos();

        bool is_calibrado(){return calibrado;}
        double dis_euclidiana(nodo *a, nodo *b);
        void medir();
        void drawcuerpo(vector <nodo> &marcas, Mat &frame);
        virtual ~cuerpo();
    protected:
        void copiar_nodo(nodo *a, nodo *b);

};

#endif // CUERPO_H
