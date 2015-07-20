#include "cuerpo.h"

cuerpo::cuerpo()
{
    calibrado=false;
    marcas.resize(13);
    //actualizaciones.resize(13);
    //<pie_d, pie_i, rod_d, rod_i, cad_d, cad_i, hom_d, hom_i, cab, codo_d, codo_i,mano_d, mano_i>
    pie_d = &marcas[0];
    pie_i = &marcas[1];
    rod_d = &marcas[2];
    rod_i = &marcas[3];
    cad_d = &marcas[4];
    cad_i = &marcas[5];
    hom_d = &marcas[6];
    hom_i = &marcas[7];
    cab = &marcas[8];
    codo_d = &marcas[9];
    codo_i = &marcas[10];
    mano_d = &marcas[11];
    mano_i = &marcas[12];
    ds = 100;
}

void cuerpo::copiar_nodo(nodo *a, nodo *b){
    a->setx(b->getx());
    a->sety(b->gety());
    a->setHSVmin(b->getHSVmin());
    a->setHSVmax(b->getHSVmax());
}

void cuerpo::calibrar(vector <nodo> &_marcas){
    if(_marcas.size() == 13){
        //double sum_avg_x=0, sum_avg_y=0;
        ptr_marcas.resize(0);
        for(size_t i=0; i<_marcas.size(); ++i){
            ptr_marcas.push_back(&_marcas[i]);
        }
        ord_ptr_marcas();
        /*
        for(size_t i=0; i<_marcas.size(); ++i){
            sum_avg_x += _marcas[i].getx();
            sum_avg_y += _marcas[i].gety();
        }
        avg_x = sum_avg_x/_marcas.size();
        avg_y = sum_avg_y/_marcas.size();
        */
        //El 90% de las posiciones pies y rodillas estan por debajo de avg_x
        //El 90% de las poiiciones la cabeza hombros y codos estan por encima de avg_x
        //El 90% de las veces el avg_y divide al cuerpo en dos bilateralmente similares
        identificar_marcas();
        if(!is_calibrado()){
            medir();
            calibrado=true;
        }
    }
}

cuerpo::~cuerpo()
{
    //dtor
}

void cuerpo::ord_ptr_marcas(){
    nodo *temp;
    #pragma parallel for
    for(size_t i=0; i<ptr_marcas.size()-1; ++i){
        for(size_t j=0; j<ptr_marcas.size()-1; ++j){
            if(ptr_marcas[j]->gety() < ptr_marcas[j+1]->gety()){
                temp = ptr_marcas[j+1];
                ptr_marcas[j+1] = ptr_marcas[j];
                ptr_marcas[j] = temp;
            }
        }
    }
}

void cuerpo::identificar_marcas(){
    detect_pies();
    detect_rodillas();
    detect_caderas();//si no fue calibrado por primera vez detectara también manos
    detect_hombros();
    detect_cab();
    detect_codos();
    detect_manos();
}
//Asumiendo que el 90% de las posiciones el pie no supera la altura de la rodilla
void cuerpo::detect_pies(){
    if(ptr_marcas[0]->getx() < ptr_marcas[1]->getx()){
        copiar_nodo(pie_d, ptr_marcas[0]);
        copiar_nodo(pie_i, ptr_marcas[1]);
    }else{
        copiar_nodo(pie_d, ptr_marcas[1]);
        copiar_nodo(pie_i, ptr_marcas[0]);
    }
    ptr_marcas.erase(ptr_marcas.begin(),ptr_marcas.begin()+2);
}
//Asumiendo que el 90% de las posiciones la rodilla no supera la altura de la cadera
void cuerpo::detect_rodillas(){
        if(ptr_marcas[0]->getx() < ptr_marcas[1]->getx()){
            copiar_nodo(rod_d, ptr_marcas[0]);
            copiar_nodo(rod_i, ptr_marcas[1]);
        }else{
            copiar_nodo(rod_d, ptr_marcas[1]);
            copiar_nodo(rod_i, ptr_marcas[0]);
        }
        ptr_marcas.erase(ptr_marcas.begin(),ptr_marcas.begin()+2);
}
//Asumiendo que en el 90% de ocasiones las manos estan a la altura de las caderas en diferentes escalas x
void cuerpo::detect_caderas(){
    nodo *temp;
    //identificamos en base a x los 4 valores problema
    if(!is_calibrado()){
        //#pragma parallel for
        for(size_t i=0; i<3; ++i){
            for(size_t j=0; j<3; ++j){
                if(ptr_marcas[j]->getx() > ptr_marcas[j+1]->getx()){
                    temp = ptr_marcas[j+1];
                    ptr_marcas[j+1] = ptr_marcas[j];
                    ptr_marcas[j] = temp;
                }
            }
        }
        //<pie_d, pie_i, rod_d, rod_i, cad_d, cad_i, hom_d, hom_i, cab, codo_d, codo_i,mano_d, mano_i>
        copiar_nodo(cad_d, ptr_marcas[1]);
        copiar_nodo(cad_i, ptr_marcas[2]);
        copiar_nodo(mano_d, ptr_marcas[0]);
        copiar_nodo(mano_i, ptr_marcas[3]);
        ptr_marcas.erase(ptr_marcas.begin(),ptr_marcas.begin()+4);
    }else{
        int cd=-1,ci=-1;
        #pragma parallel for
        for(size_t i=0; i<3; ++i){
            if(dis_euclidiana(rod_d,ptr_marcas[i]) < rod_cad_d+ds && dis_euclidiana(rod_d,ptr_marcas[i]) > rod_cad_d-ds){
                if(cd>-1){
                    if(dis_euclidiana(cad_d,ptr_marcas[cd]) > dis_euclidiana(cad_d,ptr_marcas[i]))
                        cd=i;
                }else{
                    cd=i;
                }
            }
            if(dis_euclidiana(rod_i,ptr_marcas[i]) < rod_cad_i+ds && dis_euclidiana(rod_i,ptr_marcas[i]) > rod_cad_i-ds){
                if(ci>-1){
                    if(dis_euclidiana(cad_i,ptr_marcas[ci]) > dis_euclidiana(cad_i,ptr_marcas[i]))
                        ci=i;
                }else{
                    ci=i;
                }
            }
        }
        if(cd>=ci){
            if(cd>-1){
                copiar_nodo(cad_d, ptr_marcas[cd]);
                ptr_marcas.erase(ptr_marcas.begin()+cd);
            }
            if(ci>-1){
                copiar_nodo(cad_i, ptr_marcas[ci]);
                ptr_marcas.erase(ptr_marcas.begin()+ci);
            }
        }else{
            copiar_nodo(cad_i, ptr_marcas[ci]);
            ptr_marcas.erase(ptr_marcas.begin()+ci);
            if(cd>-1){
                copiar_nodo(cad_d, ptr_marcas[cd]);
                ptr_marcas.erase(ptr_marcas.begin()+cd);
            }
        }
    }
}
void cuerpo::detect_hombros(){
    nodo *temp;
    if(!is_calibrado()){
        //quedan 5 marcas: codos, hombros y cabeza
        //<pie_d, pie_i, rod_d, rod_i, cad_d, cad_i, hom_d, hom_i, cab, codo_d, codo_i,mano_d, mano_i>
        if(ptr_marcas[2]->getx() < ptr_marcas[3]->getx()){
            copiar_nodo(hom_d, ptr_marcas[2]);
            copiar_nodo(hom_i, ptr_marcas[3]);
        }else{
            copiar_nodo(hom_d, ptr_marcas[3]);
            copiar_nodo(hom_i, ptr_marcas[2]);
        }
        ptr_marcas.erase(ptr_marcas.begin()+2,ptr_marcas.begin()+4);
    }else{
        //quedan 7 marcas:  manos, codos, hombros y cabeza
        //<pie_d, pie_i, rod_d, rod_i, cad_d, cad_i, hom_d, hom_i, cab, codo_d, codo_i,mano_d, mano_i>
        int hd=-1,hi=-1;
        //#pragma parallel for
        for(size_t i=0; i<ptr_marcas.size(); ++i){
            if(dis_euclidiana(cad_d,ptr_marcas[i]) < cad_hom_d+ds && dis_euclidiana(cad_d,ptr_marcas[i]) > cad_hom_d-ds){
                if(hd>-1){
                    if(dis_euclidiana(hom_d,ptr_marcas[hd]) > dis_euclidiana(hom_d,ptr_marcas[i]))
                        hd=i;
                }else{
                    hd=i;
                }
            }
            if(dis_euclidiana(cad_i,ptr_marcas[i]) < cad_hom_i+ds && dis_euclidiana(cad_i,ptr_marcas[i]) > cad_hom_i-ds){
                if(hi>-1){
                    if(dis_euclidiana(hom_i,ptr_marcas[hi]) > dis_euclidiana(hom_i,ptr_marcas[i]))
                        hi=i;
                }else{
                    hi=i;
                }
            }
        }
        if(hd>=hi){
            if(hd>-1){
                copiar_nodo(hom_d, ptr_marcas[hd]);
                ptr_marcas.erase(ptr_marcas.begin()+hd);
            }
            if(hi>-1){
                copiar_nodo(hom_i, ptr_marcas[hi]);
                ptr_marcas.erase(ptr_marcas.begin()+hi);
            }
        }else{
            copiar_nodo(hom_i, ptr_marcas[hi]);
            ptr_marcas.erase(ptr_marcas.begin()+hi);
            if(hd>-1){
                copiar_nodo(hom_d, ptr_marcas[hd]);
                ptr_marcas.erase(ptr_marcas.begin()+hd);
            }
        }
    }
}
void cuerpo::detect_cab(){
    if(!is_calibrado()){
        //quedan 3 marcas: codos, y cabeza
        //<pie_d, pie_i, rod_d, rod_i, cad_d, cad_i, hom_d, hom_i, cab, codo_d, codo_i,mano_d, mano_i>
        copiar_nodo(cab, ptr_marcas[2]);
        ptr_marcas.erase(ptr_marcas.begin()+2);
    }else{
        //quedan 5 marcas: manos, codos, y cabeza
        //<pie_d, pie_i, rod_d, rod_i, cad_d, cad_i, hom_d, hom_i, cab, codo_d, codo_i,mano_d, mano_i>
        int c=-1;
        #pragma parallel for
        for(size_t i=0; i<ptr_marcas.size(); ++i){
            if(dis_euclidiana(hom_d,ptr_marcas[i]) < cab_homb+ds && dis_euclidiana(hom_d,ptr_marcas[i]) > cab_homb-ds &&
               dis_euclidiana(hom_i,ptr_marcas[i]) < cab_homb+ds && dis_euclidiana(hom_i,ptr_marcas[i]) > cab_homb-ds){
                if(c>-1){
                    if(dis_euclidiana(cab,ptr_marcas[c]) > dis_euclidiana(cab,ptr_marcas[i]))
                        c=i;
                }else{
                    c=i;
                }
            }
        }
        if(c>-1){
            copiar_nodo(&marcas[8], ptr_marcas[c]);
            ptr_marcas.erase(ptr_marcas.begin()+c);
        }
    }
}

void cuerpo::detect_codos(){
    if(!is_calibrado()){
        //quedan 2 marcas: codos
        //<pie_d, pie_i, rod_d, rod_i, cad_d, cad_i, hom_d, hom_i, cab, codo_d, codo_i,mano_d, mano_i>
        if(ptr_marcas[0]->getx() < ptr_marcas[1]->getx()){
            copiar_nodo(codo_d, ptr_marcas[0]);
            copiar_nodo(codo_i, ptr_marcas[1]);
        }else{
            copiar_nodo(codo_d, ptr_marcas[1]);
            copiar_nodo(codo_i, ptr_marcas[0]);
        }
        ptr_marcas.resize(0);
    }else{
        //quedan 4 marcas: manos y codos
        //<pie_d, pie_i, rod_d, rod_i, cad_d, cad_i, hom_d, hom_i, cab, codo_d, codo_i,mano_d, mano_i>
        int cd=-1,ci=-1;
        #pragma parallel for
        for(size_t i=0; i<ptr_marcas.size(); ++i){
            if(dis_euclidiana(hom_d,ptr_marcas[i]) < hom_codo_d+ds && dis_euclidiana(hom_d,ptr_marcas[i]) > hom_codo_d-ds){
                if(cd>-1){
                    if(dis_euclidiana(codo_d,ptr_marcas[cd]) > dis_euclidiana(codo_d,ptr_marcas[i]))
                        cd=i;
                }else{
                    cd=i;
                }
            }
            if(dis_euclidiana(hom_i,ptr_marcas[i]) < hom_codo_i+ds && dis_euclidiana(hom_i,ptr_marcas[i]) > hom_codo_i-ds){
                if(ci>-1){
                    if(dis_euclidiana(codo_i,ptr_marcas[ci]) > dis_euclidiana(codo_i,ptr_marcas[i]))
                        ci=i;
                }else{
                    ci=i;
                }
            }
        }
        if(cd>=ci){
            if(cd>-1){
                copiar_nodo(codo_d, ptr_marcas[cd]);
                ptr_marcas.erase(ptr_marcas.begin()+cd);
            }
            if(ci>-1){
                copiar_nodo(codo_i, ptr_marcas[ci]);
                ptr_marcas.erase(ptr_marcas.begin()+ci);
            }
        }else{
            copiar_nodo(codo_i, ptr_marcas[ci]);
            ptr_marcas.erase(ptr_marcas.begin()+ci);
            if(cd>-1){
                copiar_nodo(codo_d, ptr_marcas[cd]);
                ptr_marcas.erase(ptr_marcas.begin()+cd);
            }
        }
    }
}
void cuerpo::detect_manos(){
    if(is_calibrado()){
        //quedan 2 marcas: manos
        //<pie_d, pie_i, rod_d, rod_i, cad_d, cad_i, hom_d, hom_i, cab, codo_d, codo_i,mano_d, mano_i>
        if(ptr_marcas[0]->getx() < ptr_marcas[1]->getx()){
            copiar_nodo(mano_d, ptr_marcas[0]);
            copiar_nodo(mano_i, ptr_marcas[1]);
        }else{
            copiar_nodo(mano_d, ptr_marcas[1]);
            copiar_nodo(mano_i, ptr_marcas[0]);
        }
        ptr_marcas.resize(0);
    }
}

double cuerpo::dis_euclidiana(nodo *a, nodo *b){
	double d = sqrt((a->getx() - b->getx())*(a->getx() - b->getx()) + (a->gety() - b->gety())*(a->gety() - b->gety()));
	return d;
}

void cuerpo::medir(){
    pie_rod = (dis_euclidiana(pie_i,rod_i) + dis_euclidiana(pie_d,rod_d))/2;
    rod_cad_d = dis_euclidiana(rod_d,cad_d);
    rod_cad_i = dis_euclidiana(rod_i,cad_i);
    cad_cad = dis_euclidiana(cad_i,cad_d);
    cad_hom_d = dis_euclidiana(cad_d,hom_d);
    cad_hom_i = dis_euclidiana(cad_i,hom_i);
    hom_hom = dis_euclidiana(hom_d,hom_i);
    hom_codo_d = dis_euclidiana(hom_d,codo_d);
    hom_codo_i = dis_euclidiana(hom_i,codo_i);
    codo_mano = (dis_euclidiana(codo_i,mano_i) + dis_euclidiana(codo_d,mano_d))/2;
    cab_homb = (dis_euclidiana(hom_i,cab) + dis_euclidiana(hom_d,cab))/2;
}

void cuerpo::drawcuerpo(vector <nodo> &_marcas, Mat &frame){
    const Scalar white = Scalar(255);
    calibrar(_marcas);
    cv::Size frameSize(640, 480);
    frame = Mat::zeros(frameSize, CV_8UC1);

    if(is_calibrado()){

        #pragma parallel for
        for(size_t i=0; i<marcas.size(); ++i){
            ellipse( frame, cv::Point(marcas.at(i).getx(),marcas.at(i).gety()), Size(10,10), 0, 0, 360, white, -1, 8, 0 );
        }
        ellipse( frame, cv::Point(cab->getx(),cab->gety()), Size(25,50), 0, 0, 360, white, -1, 8, 0 );
        line(frame, Point(pie_d->getx(),pie_d->gety()),Point(rod_d->getx() ,rod_d->gety()), white, 5, 8, 0);
        line(frame, Point(pie_i->getx(),pie_i->gety()),Point(rod_i->getx() ,rod_i->gety()), white, 5, 8, 0);
        line(frame, Point(rod_d->getx(),rod_d->gety()),Point(cad_d->getx() ,cad_d->gety()), white, 5, 8, 0);
        line(frame, Point(rod_i->getx(),rod_i->gety()),Point(cad_i->getx() ,cad_i->gety()), white, 5, 8, 0);
        line(frame, Point(cad_d->getx(),cad_d->gety()),Point(hom_d->getx() ,hom_d->gety()), white, 5, 8, 0);
        line(frame, Point(cad_i->getx(),cad_i->gety()),Point(hom_i->getx() ,hom_i->gety()), white, 5, 8, 0);
        line(frame, Point(hom_d->getx(),hom_d->gety()),Point(codo_d->getx() ,codo_d->gety()), white, 5, 8, 0);
        line(frame, Point(hom_i->getx(),hom_i->gety()),Point(codo_i->getx() ,codo_i->gety()), white, 5, 8, 0);
        line(frame, Point(codo_d->getx(),codo_d->gety()),Point(mano_d->getx() ,mano_d->gety()), white, 5, 8, 0);
        line(frame, Point(codo_i->getx(),codo_i->gety()),Point(mano_i->getx() ,mano_i->gety()), white, 5, 8, 0);
        line(frame, Point(hom_d->getx(),hom_d->gety()),Point(hom_i->getx() ,hom_i->gety()), white, 5, 8, 0);
        line(frame, Point(cad_d->getx(),cad_d->gety()),Point(cad_i->getx() ,cad_i->gety()), white, 5, 8, 0);
    }
}
