#pragma once
#include <cmath>

// Football Billiards: independent rules and fixed-step simulation.
struct FutbolBillar {
    struct Vec { float x, y; };
    static constexpr float radi = 0.16f;
    static constexpr float limitX = 4.84f;
    static constexpr float limitY = 2.34f;
    static constexpr float porteria = 0.85f;
    Vec p[11]{}, v[11]{};
    int gols[2]{}, torn=1, seleccio=1, guanyador=0, camera=2;
    float angle=0, forca=0, acumulador=0, esperaGol=0, temps=0;
    bool moviment=false, carregant=false, bloqueig=true;
    bool qAnt=true, eAnt=true, rAnt=true;
    bool ratoliCarrega=false;
    static constexpr float fregament=1.05f;
    static constexpr float rebotBanda=0.70f;
    static constexpr float rebotBoles=0.84f;
    int ultimGol=0;
    static float llarg(Vec a) { return std::sqrt(a.x*a.x+a.y*a.y); }
    bool disponible() const { return !moviment && !carregant && esperaGol<=0 && guanyador==0; }
    void formacio() {
        p[0]={0,0};
        const Vec pos[5]={{-3.95f,0},{-2.65f,-1.35f},{-2.65f,1.35f},{-0.95f,-0.7f},{-0.95f,0.7f}};
        for(int i=0;i<5;++i){p[i+1]=pos[i];p[i+6]={-pos[i].x,-pos[i].y};}
        for(auto &b:v)b={0,0};
        moviment=false; carregant=false; ratoliCarrega=false; forca=0; acumulador=0; bloqueig=true;
        seleccio=torn==1?4:9; angle=std::atan2(-p[seleccio].y,-p[seleccio].x);
    }
    void reinicia(){*this=FutbolBillar();formacio();}
    void pausa(){carregant=false;ratoliCarrega=false;forca=0;bloqueig=true;qAnt=eAnt=rAnt=true;}
    bool comencaRatoli(){
        if(!disponible())return false;
        carregant=true;ratoliCarrega=true;forca=0;return true;
    }
    void forcaRatoli(float fraccio){
        if(ratoliCarrega && carregant)forca=fraccio<0?0:(fraccio>1?1:fraccio);
    }
    int dispara(){
        carregant=false;ratoliCarrega=false;
        if(forca<0.025f){forca=0;return 0;}
        moviment=true;acumulador=0;
        const float velocitat=1.15f+6.65f*forca;
        v[seleccio]={std::cos(angle)*velocitat,std::sin(angle)*velocitat};
        return 8;
    }
    int acabaRatoli(){return ratoliCarrega && carregant ? dispara():0;}
    void selecciona(int dir){
        if(!disponible())return;
        int inici=torn==1?1:6;
        seleccio=inici+(seleccio-inici+dir+5)%5;
        angle=std::atan2(p[0].y-p[seleccio].y,p[0].x-p[seleccio].x);
    }
    void canviaTorn(){torn=3-torn;seleccio=torn==1?1:6;angle=std::atan2(p[0].y-p[seleccio].y,p[0].x-p[seleccio].x);forca=0;}
    void marca(int equip){
        ++gols[equip-1];ultimGol=equip;esperaGol=1.7f;
        for(auto &b:v)b={0,0};
        moviment=false;carregant=false;ratoliCarrega=false;forca=0;bloqueig=true;
        if(gols[equip-1]>=5)guanyador=equip;
        torn=3-equip;
    }
    // Returns a bit mask: 1 ball contact, 2 wall, 4 goal, 8 shot.
    int pas(float dt){
        int so=0;
        for(int i=0;i<11;++i){
            float speed=llarg(v[i]);
            if(speed>0){float t=speed/fregament; if(t>dt)t=dt;
                float d=speed*t-0.5f*fregament*t*t;
                p[i].x+=v[i].x/speed*d;p[i].y+=v[i].y/speed*d;
                float nou=speed-fregament*t;if(nou<0.025f)nou=0;
                v[i].x*=nou/speed;v[i].y*=nou/speed;
            }
            // Only the football can cross a goal mouth. Team balls rebound.
            if(i==0 && std::fabs(p[i].y)<porteria-radi && std::fabs(p[i].x)>=limitX){marca(p[i].x>0?1:2);return so|4;}
            float lx=limitX-radi,ly=limitY-radi;
            bool boca=i==0 && std::fabs(p[i].y)<porteria-radi;
            if(!boca && std::fabs(p[i].x)>lx){float s=p[i].x>0?1.f:-1.f;p[i].x=s*lx;if(v[i].x*s>0){v[i].x*=-rebotBanda;so|=2;}}
            if(std::fabs(p[i].y)>ly){float s=p[i].y>0?1.f:-1.f;p[i].y=s*ly;if(v[i].y*s>0){v[i].y*=-rebotBanda;so|=2;}}
        }
        // Equal-mass sphere impulses with positional correction.
        for(int sweep=0;sweep<3;++sweep)for(int i=0;i<11;++i)for(int j=i+1;j<11;++j){
            float dx=p[j].x-p[i].x,dy=p[j].y-p[i].y,d=std::sqrt(dx*dx+dy*dy);
            if(d>=2*radi)continue;
            float nx=d>0.00001f?dx/d:1.f,ny=d>0.00001f?dy/d:0.f;
            float corr=(2*radi-d)*0.5f;
            p[i].x-=nx*corr;p[i].y-=ny*corr;p[j].x+=nx*corr;p[j].y+=ny*corr;
            float rel=(v[j].x-v[i].x)*nx+(v[j].y-v[i].y)*ny;
            if(rel<0){float imp=-0.5f*(1.f+rebotBoles)*rel;v[i].x-=imp*nx;v[i].y-=imp*ny;v[j].x+=imp*nx;v[j].y+=imp*ny;if(imp>0.10f)so|=1;}
        }
        return so;
    }
    int actualitza(float dt,bool espai,bool esquerra,bool dreta,bool q,bool e,bool r){
        int so=0;temps+=dt;
        if(r && !rAnt){reinicia();rAnt=true;return 0;}rAnt=r;
        if(esperaGol>0){esperaGol-=dt;if(esperaGol<=0 && !guanyador){esperaGol=0;formacio();}qAnt=q;eAnt=e;bloqueig=espai;return 0;}
        if(guanyador)return 0;
        if(q&&!qAnt)selecciona(-1);
        if(e&&!eAnt)selecciona(1);
        qAnt=q;eAnt=e;
        if(!moviment && !ratoliCarrega){
            if(esquerra)angle+=1.45f*dt;
            if(dreta)angle-=1.45f*dt;
            if(espai && !bloqueig && !carregant){carregant=true;forca=0.12f;}
            if(espai && carregant){forca+=dt/0.95f;if(forca>1)forca=1;}
            if(!espai && carregant)so|=dispara();
        }
        bloqueig=espai;
        if(moviment){
            acumulador+=dt;const float pasFix=1.f/300.f;
            while(acumulador>=pasFix && moviment){acumulador-=pasFix;so|=pas(pasFix);}
            bool quiet=true;for(auto b:v)if(llarg(b)>0.025f)quiet=false;
            if(quiet && moviment){for(auto &b:v)b={0,0};moviment=false;canviaTorn();}
        }
        return so;
    }
};
