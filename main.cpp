#include "graphics.hpp"
#include <vector>
#include <fstream>
#include <sstream>

using namespace genv;
using namespace std;

const int w=700,h=700,
          blocksize=20, //rács négyzet mérete (pixelben) - a program 20 köré épült, ennek megváltoztatása világkatasztrófát eredményez (és még a program sem fog mûködni)
          falvastag=2,  //pályaszéli oldalfal vastagsága (blockban)
          beltav=4;     //belso fal oldalfaltól való távolsága (blockban)

struct pont
{
    int x,y;
};

struct szin
{
    int r,g,b;
};

struct block
{
    pont koord;
    char status='s';    //s=szabad, f=fal, t=test, (p=volt test), a=alma
};

bool operator==(const pont &a, const pont &b)
{
    return (a.x==b.x and a.y==b.y);
}

struct Textpanel
{
    protected:
        pont koord;
        szin txtcolor;
        int szel,mag;
        string szoveg;

    public:
    Textpanel(string s)
    {
        szel=w/3;
        mag=h/10;
        koord.x=w/2-szel/2;
        koord.y=h-(mag*3.5);
        szoveg=s;
        txtcolor.r=255;
        txtcolor.g=120;
        txtcolor.b=0;
    }

    virtual void rajzol()
    {
        gout.load_font("LiberationMono-Bold.ttf",w/20);
        gout << color(67,67,67) << move_to(koord.x,koord.y) << box(szel,mag)
             << color(100,100,100) << move_to(koord.x+5,koord.y+5) << box(szel-10,mag-10)

             << color(txtcolor.r,txtcolor.g,txtcolor.b)
             << move_to(koord.x+(szel/2)-(gout.twidth(szoveg)/2),koord.y+(mag/2)-((gout.cascent()+gout.cdescent())/2))
             << text(szoveg);
    }

    bool rajta(int cx, int cy)
    {
        return (cx>koord.x and cx<koord.x+szel and cy>koord.y and cy<koord.y+mag);
    }

    void colorchange(szin becolor)
    {
        txtcolor=becolor;
    }
};

struct Title:public Textpanel
{
    protected:
        string alcim;

    public:
    Title(string s, string als): Textpanel(s)
    {
        koord.x=w/7;
        koord.y=h/7;
        szel=w/7*5;
        mag=h/5;
        alcim=als;
        txtcolor.r=0;
        txtcolor.g=255;
        txtcolor.b=0;
    }

    void rajzol() override
    {
        gout.load_font("LiberationMono-Bold.ttf",w/14);
        gout << color(100,0,0) << move_to(koord.x,koord.y) << box(szel,mag)
             << color(160,0,0) << move_to(koord.x+10,koord.y+10) << box(szel-20,mag-20)

             << color(txtcolor.r,txtcolor.g,txtcolor.b) << move_to(w/2-(gout.twidth(szoveg)/2),koord.y+(mag/2)-5-(gout.cascent()+gout.cdescent())/2)
             << text(szoveg);
        int txtmag=gout.cascent()+gout.cdescent();
        gout.load_font("LiberationMono-Bold.ttf",w/50);
        gout << move_to(w/2-(gout.twidth(alcim)/2),koord.y+(mag/2)+(txtmag/2)-(gout.cascent()+gout.cdescent())/2)
             << text(alcim);
    }
};

struct Gomb:public Textpanel
{
    protected:
        bool jelolt=false;

    public:
    Gomb(int x_, int y_, string s, int r_, int g_, int b_): Textpanel(s)
    {
        szel=w/5;
        mag=h/10;
        koord.x=x_-szel/2;
        koord.y=y_-mag/2;
        txtcolor.r=r_;
        txtcolor.g=g_;
        txtcolor.b=b_;
    }

    void rajzol() override
    {
        gout.load_font("LiberationMono-Bold.ttf",w/28);
        if(jelolt)
        {
            gout << color(txtcolor.r,txtcolor.g,txtcolor.b);
        }
        else
        {
            gout << color(67,67,67);
        }
        gout << move_to(koord.x,koord.y) << box(szel,mag)
             << color(100,100,100) << move_to(koord.x+5,koord.y+5) << box(szel-10,mag-10)

             << color(txtcolor.r,txtcolor.g,txtcolor.b)
             << move_to(koord.x+(szel/2)-(gout.twidth(szoveg)/2),koord.y+(mag/2)-((gout.cascent()+gout.cdescent())/2))
             << text(szoveg);
    }

    void jelolvalt(bool b)
    {
        jelolt=b;
    }

    int difficulty()
    {
        if(szoveg=="Hard")
            return 200;
        else if(szoveg=="Easy")
            return 400;
        else
            return 300;
    }

    szin colorker()
    {
        return txtcolor;
    }
};

struct Racs
{
    private:
        vector<vector<block>> grid;

    public:
    Racs()
    {
        grid=vector<vector<block>>(h/blocksize,vector<block>(w/blocksize));
        for(size_t i=0;i<grid.size();i++)
        {
            for(size_t j=0;j<grid[i].size();j++)
            {
                if(j==0)
                {
                    grid[i][j].koord.x=0;
                }
                else
                {
                    grid[i][j].koord.x=grid[i][j-1].koord.x+blocksize;
                }
                if(i==0)
                {
                    grid[i][j].koord.y=0;
                }
                else
                {
                    grid[i][j].koord.y=grid[i-1][j].koord.y+blocksize;
                }

                if(i<falvastag || j<falvastag || i>grid.size()-falvastag-1 || j>grid[i].size()-falvastag-1 ||  //oldalfalak
                   (i==falvastag+beltav && j>=falvastag+beltav && j<=grid[i].size()-(falvastag+beltav)-1) ||   // belsõ
                   (j==grid[i].size()/2 && i>=falvastag+beltav && i<=grid.size()-(falvastag+beltav)-1))        //  fal
                {
                    grid[i][j].status='f';
                }
            }
        }
    }

    void hatter(canvas &hatter, canvas fu, canvas fal)
    {
        for(size_t i=0;i<grid.size();i++)
        {
            for(size_t j=0;j<grid[i].size();j++)
            {
                if(grid[i][j].status=='f')
                {
                    hatter << stamp(fal,grid[i][j].koord.x,grid[i][j].koord.y);
                }
                else
                {
                    hatter << stamp(fu,grid[i][j].koord.x,grid[i][j].koord.y);
                }
            }
        }
    }

    int xblockszam()
    {
        return grid[0].size();
    }

    int yblockszam()
    {
        return grid.size();
    }

    void testclear()
    {
        for(size_t i=0;i<grid.size();i++)
        {
            for(size_t j=0;j<grid[i].size();j++)
            {
                if(grid[i][j].status=='t')
                {
                    grid[i][j].status='p';
                }
                else if(grid[i][j].status=='p')
                {
                    grid[i][j].status='s';
                }
            }
        }
    }

    void testadd(pont testpoz)
    {
        for(size_t i=0;i<grid.size();i++)
        {
            for(size_t j=0;j<grid[i].size();j++)
            {
                if(grid[i][j].koord==testpoz)
                {
                    grid[i][j].status='t';
                }
            }
        }
    }

    char detect(pont fejpoz, char irany)
    {
        block* nextblock;
        bool volt=false;
        for(size_t i=0;i<grid.size();i++)
        {
            for(size_t j=0;j<grid[i].size();j++)
            {
                if(grid[i][j].koord==fejpoz && !volt)
                {
                    if(irany=='j')
                    {
                        nextblock=&grid[i][j+1];
                    }
                    else if(irany=='f')
                    {
                        nextblock=&grid[i-1][j];
                    }
                    else if(irany=='l')
                    {
                        nextblock=&grid[i+1][j];
                    }
                    else
                    {
                        nextblock=&grid[i][j-1];
                    }
                    volt=true;
                }
            }
        }
        if(nextblock->status=='a')
        {
            nextblock->status='s';
            return 'a';
        }
        return nextblock->status;
    }

    void ghostmode(canvas c)
    {
        for(size_t i=0;i<grid.size();i++)
        {
            for(size_t j=0;j<grid[i].size();j++)
            {
                if(grid[i][j].status=='p')
                {
                    gout << stamp(c,grid[i][j].koord.x,grid[i][j].koord.y);
                }
            }
        }
    }

    pont almakoord()
    {
        int i=rand()%grid.size(),j=rand()%grid[0].size();
        while(grid[i][j].status!='s')
        {
            i=rand()%grid.size();
            j=rand()%grid[0].size();
        }
        grid[i][j].status='a';
        return grid[i][j].koord;
    }
};

struct Fej
{
    private:
        pont koord;
        char irany='j'; //j=jobb, b=bal, f=fel, l=le
        canvas textura;

    public:
    Fej(int xblock, int yblock, canvas c)
    {
        koord.x=(xblock/2+1)*20;
        koord.y=(yblock*20)-(falvastag*20+(beltav/2)*20);
        textura=c;
    }

    void rajzol()
    {
        gout << stamp(textura,koord.x,koord.y);
    }

    pont pozicio()
    {
        return koord;
    }

    char iranyker()
    {
        return irany;
    }

    void mozog(char c)
    {
        irany=c;
        if(irany=='j')
        {
            koord.x+=blocksize;
        }
        else if(irany=='f')
        {
            koord.y-=blocksize;
        }
        else if(irany=='l')
        {
            koord.y+=blocksize;
        }
        else
        {
            koord.x-=blocksize;
        }
    }

    void texreplace(canvas c)
    {
        textura=c;
    }
};

struct Test
{
    private:
        pont koord;
        Test* follow;
        canvas textura;

    public:
    Test(int x_, int y_, Test* p, canvas c)
    {
        koord.x=x_;
        koord.y=y_;
        follow=p;
        textura=c;
    }

    void rajzol()
    {
        gout << stamp(textura,koord.x,koord.y);
    }

    pont pozicio()
    {
        return koord;
    }

    void mozog(pont fejpoz)
    {
        if(follow==0)
        {
            koord.x=fejpoz.x;
            koord.y=fejpoz.y;
        }
        else
        {
            koord.x=follow->koord.x;
            koord.y=follow->koord.y;
        }
    }
};

struct Alma
{
    private:
        pont koord;
        canvas textura;
        bool palyan=false;

    public:
    Alma(canvas c)
    {
        textura=c;
    }

    void rajzol()
    {
        if(palyan)
        {
            gout << stamp(textura,koord.x,koord.y);
        }
    }

    void makealma(pont bekoord)
    {
        koord=bekoord;
        palyan=true;
    }

    void megesz()
    {
        palyan=false;
    }
};

void import(string fajl, canvas &c)
{
    ifstream be(fajl);
    vector<vector<szin>> kep;
    int szel, mag;
    be >> szel >> mag;
    c.open(szel,mag);
    kep=vector<vector<szin>>(mag,vector<szin>(szel));
    for(size_t i=0;i<kep.size();i++)
    {
        for(size_t j=0;j<kep[i].size();j++)
        {
            be >> kep[i][j].r >> kep[i][j].g >> kep[i][j].b;
            c << move_to(j,i) << color(kep[i][j].r,kep[i][j].g,kep[i][j].b) << dot;
        }
    }
    be.close();
}

void gombrajzol(Title cim, vector<Gomb*> gombok, Textpanel start)
{
    cim.rajzol();
    for(size_t i=0;i<gombok.size();i++)
    {
        gombok[i]->rajzol();
    }
    start.rajzol();
    gout << refresh;
}

void rajzol(canvas hatter, Fej fej, vector<Test*> testek, Alma alma, int pontszam)
{
    gout.load_font("LiberationMono-Bold.ttf",20);
    stringstream ss;
    gout << stamp(hatter,0,0);
    for(size_t i=0;i<testek.size();i++)
    {
        testek[i]->rajzol();
    }
    alma.rajzol();
    fej.rajzol();
    ss << "Pontok: " << pontszam;
    gout << color(0,255,0) << move_to((w/2)-gout.twidth(ss.str())/2,blocksize/2) << text(ss.str())
         << refresh;
}

int main()
{
    bool gamestart=true, gameover=false;
    char inputirany='j', fejstatus;
    int almakor=0, pontszam=0;
    const int almatime=8;  //almatime: alma létrehozásának gyakorisága (lépésben) (pl.: megevéstõl számított 8. lépésben)
    gout.open(w,h);

    canvas hatter,fejle,fejfel,fejjobb,fejbal,fejdead,test,fal,fu,almatex;
    import("fej2le.kep",fejle);          //
    import("fej2fel.kep",fejfel);        //
    import("fej2jobb.kep",fejjobb);      //
    import("fej2bal.kep",fejbal);        //   Textúrák
    import("fej2dead.kep",fejdead);      //  importálása
    import("test.kep",test);             //
    import("fal.kep",fal);               //
    import("fu.kep",fu);                 //
    import("alma.kep",almatex);          //

    Alma alma(almatex);                  //alma létrehozása
    Racs racs;                           //
    hatter.open(w,h);                    //  Rács és háttér létrehozása
    racs.hatter(hatter,fu,fal);          //
    vector<Test*> testek;                                                                                  //
    Fej fej(racs.xblockszam(),racs.yblockszam(),fejjobb);                                                  //                                                                                          //
    testek.push_back(new Test(fej.pozicio().x-blocksize,fej.pozicio().y,0,test));                          //  Fej és test                                                                             //  alapállapota
    testek.push_back(new Test(testek[0]->pozicio().x-blocksize,testek[0]->pozicio().y,testek[0],test));    //
    rajzol(hatter,fej,testek,alma,pontszam);                                                               //                                                                                     //

    vector<Gomb*> gombok;
    gombok.push_back(new Gomb(w/4,h/2,"Easy",0,255,0));
    gombok.push_back(new Gomb(w/2,h/2,"Normal",255,120,0));
    gombok.push_back(new Gomb(w/4*3,h/2,"Hard",255,0,0));
    Gomb* jelolt=gombok[1];
    jelolt->jelolvalt(true);
    Title cim("Epic Snake Game","or something");
    Textpanel start("Start");
    gombrajzol(cim,gombok,start);

    event ev;
    while(gin >> ev && ev.keycode!=key_escape) {
        if(gamestart)
        {
            if(ev.type==ev_mouse && ev.button==btn_left)
            {
                for(size_t i=0;i<gombok.size();i++)
                {
                    gombok[i]->jelolvalt(false);
                    if(gombok[i]->rajta(ev.pos_x,ev.pos_y))
                    {
                        jelolt=gombok[i];
                    }
                }
                jelolt->jelolvalt(true);
                start.colorchange(jelolt->colorker());
                gombrajzol(cim,gombok,start);

                if(start.rajta(ev.pos_x,ev.pos_y))
                {
                    gout.showmouse(false);
                    gamestart=false;
                }
            }
            gin.timer(jelolt->difficulty());
        }

        else if(!gamestart && !gameover)
        {
            if(ev.type==ev_timer)
            {
                almakor++;
                if(almakor==almatime)
                {
                    alma.makealma(racs.almakoord());
                }

                racs.testclear();
                for(int i=testek.size()-1;i>=0;i--)
                {
                    testek[i]->mozog(fej.pozicio());
                    racs.testadd(testek[i]->pozicio());
                }

                fejstatus=racs.detect(fej.pozicio(),inputirany);
                if(fejstatus=='f' || fejstatus=='t')
                {
                    fej.texreplace(fejdead);
                    gameover=true;
                }
                else
                {
                    fej.mozog(inputirany);
                    if(fejstatus=='a')
                    {
                        alma.megesz();
                        testek.push_back(new Test(testek[testek.size()-1]->pozicio().x,testek[testek.size()-1]->pozicio().y,testek[testek.size()-1],test));
                        almakor=0;
                        pontszam+=100;
                    }
                }

                rajzol(hatter,fej,testek,alma,pontszam);
                if(gameover)
                {
                    racs.ghostmode(test);
                    gout.showmouse(true);
                    gout << refresh;
                }
            }

            else if(ev.type==ev_key)
            {
                if(ev.keycode==key_right && fej.iranyker()!='b')
                {
                    inputirany='j';
                    fej.texreplace(fejjobb);
                }
                else if(ev.keycode==key_up && fej.iranyker()!='l')
                {
                    inputirany='f';
                    fej.texreplace(fejfel);
                }
                else if(ev.keycode==key_down && fej.iranyker()!='f')
                {
                    inputirany='l';
                    fej.texreplace(fejle);
                }
                else if(ev.keycode==key_left && fej.iranyker()!='j')
                {
                    inputirany='b';
                    fej.texreplace(fejbal);
                }
            }
        }

        else
        {
            Title ending("Snake perished!","and it's because of you...");
            Textpanel exit("Exit");
            exit.rajzol();
            ending.rajzol();
            gout << refresh;

            if(ev.type==ev_mouse && ev.button==btn_left && exit.rajta(ev.pos_x,ev.pos_y))
            {
                return 0;
            }
        }
    }
    return 0;
}
