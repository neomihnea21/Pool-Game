#include<iostream>
#include<vector>
#include<fstream>
#include<cmath>
#include<queue>
#include<stdexcept>
class outOfBounds: public std::exception{///e o exceptie care scrie un string si anuleaza actiunea, i. e. randul
    std::string context;
    public:
      outOfBounds(std::string Context_=""): context(Context_) {}
      std::string what(){
          return context;
      }
};
class Display{
    protected:
      virtual void print(std::ostream &out) const =0;
    friend std::ostream& operator<<(std::ostream &out, const Display &display);
};
std::ostream& operator<<(std::ostream &out, const Display &display){
    display.print(out);
    return out;///supraincarc scrierea in BAZA, derivate o iau de aici
}
class Vector: public Display{
    double x, y;
public:
    Vector(double x_=0, double y_=0): x(x_), y(y_) {}
    void setX(double x_){ x=x_; }
    void setY(double y_){y=y_; }
    double getX() const{
        return x;
    }
    double getY() const{
        return y;
    }
    double modul() const{
        return sqrt(x*x+y*y);
    }
    static double modul(double vx, double vy){///ca sa pot chema modulul intr-un vacuum
         return sqrt(vx*vx+vy*vy);
    }
    double argument(){
        return atan2(y, x);
    }
    void norm(){
       double mod=sqrt(x*x+y*y);
       x/=mod, y/=mod;
    }
    double dotProduct(Vector &other){ return x*other.x+y*other.y; }
    Vector times(double k){
        Vector v2(x*k, y*k);
        return v2;
    }
    Vector operator+(Vector &other){
        Vector v2(x+other.x, y+other.y);
        return v2;
    }
    Vector operator-(Vector &other){
        Vector v2(x-other.x, y-other.y);
        return v2;
    }
    void print(std::ostream &out) const override{
         out<<x<<" "<<y<<"\n";
    }
    friend std::istream& operator >>(std::istream &in, Vector &v);
    ///for some reason, the vector's custom = fucks with everything
};
std::istream& operator >>(std::istream &in, Vector &v){
    in>>v.x>>v.y;
    return in;
}
class Ball: public Display{
protected:
    int no;
    double x, y, r, m, mu;///coordinates, radius, mass, sfc
    Vector v;
    static double pi, dt, g, EPS; ///timpul nu curge diferit pentru bile diferite, si nici acceleratia gravitationala nu e alta
public:
    Ball(int no_=1, double x_=0, double y_=0, double r_=0, double m_=0, double mu_=0, Vector v_=0):
        no(no_), x(x_), y(y_), r(r_), m(m_), mu(mu_), v(v_) {}
    virtual void shootBall(double vx, double vy) {};//nu o sa fie nevoie niciodata de o bila care nu e alba, obiect sau finala
 public:
    virtual void potBall() {};//doar alba da override - restul sunt prea la fel, finala e tot o culoare
    double getX(){
        return x;
    }
    double getY() const{
        return y;
    }
    double getR() const{
        return r;
    }
    double getSpeed(){
        return v.modul();
    }
    int getNo(){
        return no;
    }
    Vector getV(){ return v;}
    void moveBall(){
        double vx=v.getX(), vy=v.getY();
        x+=vx*dt, y+=vy*dt; ///nudge the ball in its direction of motion
        vx-=(mu*g*cos(v.argument()))*dt;///you wanna increase negative speeds and decrease positive ones
        vy-=(mu*g*sin(v.argument()))*dt;
        v=Vector(vx, vy);
        if(v.modul()<EPS)
            v=Vector(0, 0);
    }
    void hitHorizCushion(){
        Vector newV(v.getX(), -v.getY());
        v=newV;
    }
    void hitVertCushion(){
        Vector newV(-v.getX(), v.getY());
        v=newV;
    }
    void collide(Ball &other){
        Vector rx(x, y), ry(other.x, other.y);///these are position vectors for the balls
        Vector centerLine=ry-rx; ///genuinely don't care about modulus, only azimuth
        Vector radicalAxis(other.y-y, x-other.x);///centerLine rotated by 90 degrees
        centerLine.norm();
        radicalAxis.norm();
        ///in the new system, x is the center line, y is the radical axis (it's orthonormal)
        double v1x=v.dotProduct(centerLine), v1y=v.dotProduct(radicalAxis);
        double v2x=other.v.dotProduct(centerLine),
               v2y=other.v.dotProduct(radicalAxis);///coefficients of the vectors in the new system
        Vector newV1=centerLine.times(v2x), newRad1=radicalAxis.times(v1y);
        Vector newV2=centerLine.times(v1x), newRad2=radicalAxis.times(v2y);
        v=newV1+newRad1, other.v=newV2+newRad2;///I can't add vectors freely, since rvalues are references and lvalues aren't
    }
    static void alterPhysicsEngine(double pi_=3.14, double g_=9.81, double eps=0.3, double dt_=0.01){
        pi=pi_, g=g_, EPS=eps, dt=dt_;
    }
    void print(std::ostream &out)const override{
        out<<x<<" "<<y<<"\n";
    }
    friend std::istream& operator >>(std::istream &in, Ball &b);
};
std::istream& operator>>(std::istream& in, Ball &b){
    in>>b.no>>b.x>>b.y>>b.r>>b.m>>b.mu>>b.v;
    return in;
}
class cueBall: public Ball{
    public:
    cueBall(int no_=0, double x_=0, double y_=0, double r_=0.15, double m_=0.15, double mu_=0.1, Vector V_=Vector(0, 0)):
      Ball(no_, x_, y_, r_, m_, mu_, V_) {}
    void shootBall(double vx, double vy) override{
        Vector vt(vx, vy);///vector de unique_pointer catre ball
        v=vt;
    }
    void potBall() override{
         std::cout<<"Ai bagat alba, se repune de la centru:";
         cueBall newCue=cueBall(0, 5, 3.5, 0.15, 0.15, 0.1, Vector(0, 0));///TODO scoate numerele magice
         *this=newCue;///ar trebui sa o schimbe
    }
};//in the end, singura bila cu metode speciale este alba
class regularBall: public Ball{
    public: 
      regularBall(int no_=0, double x_=0, double y_=0, double r_=0.15, double m_=0.15, double mu_=0.1, Vector V_=Vector(0, 0)):
      Ball(no_, x_, y_, r_, m_, mu_, V_) {}
    void potBall() override{
        std::cout<<"Ai bagat bila "<<no<<"\n";
    }
};
class Table{
    double L, l, pocketSize;
    std::vector<Ball*> v;
    int ctBalls;///NOTA: pe asta fa-l static
public:
    Table(double length=0, double width=0, double pocketSize=0, std::vector<Ball*>vt={}){
       L=length, l=width;
       v=vt;
    }
    std::pair<double, double> getSize(){
        return std::make_pair(L, l);
    }
    void shot(int vx, int vy){
         Vector momentumVector=Vector(vx, vy);
         if(momentumVector.modul()<30)
           v[0]->shootBall(vx, vy);///RUNTIME POLYMORPHISM HERE
         else{
            throw outOfBounds("Ai spart masa\n");
         }
    }
    Ball getBall(int i){
          if(i<v.size())
            return *v[i];///arunca o exceptie daca lovim bila care nu exista, dar cine s-o prinda?
          else
            throw outOfBounds("A testat o bila care nu exista\n");
    }
    void addBall(Ball *b){///asta trb sa poata fi chemata si cu cueBall
        v.push_back(b);
    }
    int getBallCount(){
        return v.size();
    }
    void runShot(){
       ctBalls=v.size();
       for(int i=0; i<ctBalls; i++){///we move the balls one at a time
           v[i]->moveBall();///UPCASTING PESTE TOT PE AICI, cand se refera la pointerul catre alba
           for(int j=0; j<ctBalls; j++){
              Vector dist=Vector(v[j]->getX()-v[i]->getX(), v[j]->getY()-v[i]->getY());
              if(dist.modul()<v[j]->getR()+v[i]->getR() && i!=j)
                 (*v[i]).collide(*v[j]);
           }
           double cx=v[i]->getX(), cy=v[i]->getY(), R=v[i]->getR();///extract coordinates, to easily check cushion ricochet
           if( (cx-R<0 && cy-R<0) || (cx-R<0 && cy+R>l)
           ||  (cx+R>L && cy+R>l) || (cx+R>L && cy-R<0) ){ ///if the center is very close to a pocket, the ball is potted
              v[i]->potBall(); ///might be RUNTIME POLYMORPHISM
           }
           if(cx<R||cx+R>L){///if it's too close to a vertical edge
             v[i]->hitVertCushion();///ricochets sideways
             //std::cout<<"A ricosat din verticala\n";
           }
           if(cy<R||cy+R>l){///all the same, ricochets
             v[i]->hitHorizCushion();
             //std::cout<<"A ricosat din orizontala\n";
           }
       }
    }
    ///rule of three for the tables
    //Table(const Table &other): L(other.L), l(other.l), pocketSize(other.pocketSize), whiteBall(other.whiteBall), v(other.v){ }

    Table operator=(Table &other){
        L=other.L, l=other.l;
        pocketSize=other.pocketSize; v=other.v;
        return *this;
    }
    ~Table(){
        L=0, l=0, pocketSize=0;
        v.clear();
    }
    void writeBalls(){
       std::cout<<"Avem bilele urmatoare:\n";
       std::cout<<"Bila alba la "<<*v[0];
       for(int i=1; i<v.size(); i++){ ///nu or sa apara bile albe pe aici
          std::cout<<"Bila "<<v[i]->getNo()<<" la "<<(*v[i]);
        }
    }
    friend std::ostream &operator <<(std::ostream &out, Table t);
};
std::ostream &operator <<(std::ostream &out, Table t){
    out<<t.L<<" "<<t.l<<"\n";
    for(int i=0; i<t.v.size(); i++){
        out<<t.v[i];
    }
    return out;
}
double Ball::pi=3.14;
double Ball::g=9.81;
double Ball::EPS=0.3;
double Ball::dt=0.01;
int main(){///TODO see what speeds we should impart
    std::ifstream fin("balls.txt");
    int ballCount; double tableLength, tableWidth;
    cueBall c1; ///o vom citi separat, ca e altfel de bila
    fin>>ballCount>>tableLength>>tableWidth;
    Table t(tableLength, tableWidth, 0);
    fin>>c1;
    t.addBall(&c1);///UPCASTING AICI
    double topSpeed=0;
    for(int i=0; i<ballCount; i++){
        regularBall b;
        fin>>b;
        t.addBall(&b);///UPCASTING AICI
        topSpeed=std::max(topSpeed, b.getSpeed());///initial configuration of the balls
    }
    ///the actual game loop
    t.writeBalls();
    int shots=0;
    while(t.getBallCount()>0&&shots<5){
       std::cout<<"Trageti in bila alba, dati puterea pe x si pe y: \n";
       double vx, vy; std::cin>>vx>>vy;
       try{
         t.shot(vx, vy);///trebuie sa copiem toata bila, ceea ce e otara lent, dar ok
       }
       catch(outOfBounds &e){
           std::cout<<e.what();
       }
       topSpeed=t.getBall(0).getSpeed();
       while(topSpeed>0){
         t.runShot();
         topSpeed=0;
         try{///ar trebui sa mearga normal desi e cam weird
           for(int i=0; i<ballCount; i++){
              topSpeed=std::max(topSpeed, t.getBall(i).getSpeed());
           }
         }
         catch(outOfBounds &e){
            std::cout<<e.what();
         }
      }
      t.writeBalls();
      shots++;
    }
    Table t2(t);///quick 2-liner to test copy constructors
    t2.writeBalls();
    return 0;
}
