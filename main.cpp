#include<iostream>
#include<vector>
#include<fstream>
#include<cmath>
#include<queue>
#include<stdexcept>
class outOfBounds: public std::exception{
    using std::exception::exception;
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
    double pi=3.14, dt=0.01, g=9.81, EPS=0.3; ///timpul nu curge diferit pentru bile diferite, si nici acceleratia gravitationala nu e alta
public:
    Ball(int no_=1, double x_=0, double y_=0, double r_=0, double m_=0, double mu_=0, Vector v_=0):
        no(no_), x(x_), y(y_), r(r_), m(m_), mu(mu_), v(v_) {}
 protected: ///singurul loc de unde se trage in bila e derivata cueBall, de-asta e protected
    virtual void shootBall(double vx, double vy) {};//nu o sa fie nevoie niciodata de o bila care nu e alba, obiect sau finala
 public:
    virtual void potBall() {};//doar alba da override - restul sunt prea la fel, finala e tot o culoare
    double getX() const{
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
    //friend std::ostream &operator <<(std::ostream &out, cueBall &c);///pentru ca face figuri cu printul din abstract, ii dam in dulcele stil clasic
};//in the end, singura bila cu metode speciale este alba
class Table{
    double L, l, pocketSize;
    cueBall whiteBall; 
    std::vector<Ball> v;
public:
    Table(double length=0, double width=0, double pocketSize=0, std::vector<Ball>vt={}){
       L=length, l=width;
       v=vt;
    }
    std::pair<double, double> getSize(){
        return std::make_pair(L, l);
    }
    void shot(int vx, int vy){
        whiteBall.shootBall(vx, vy);
    }
    Ball getBall(int i){
          if(i<v.size())
            return v[i];///arunca o exceptie daca lovim bila care nu exista, dar cine s-o prinda?
    }
    const cueBall& getWhiteBall(){
        return whiteBall;
    }
    void addBall(Ball b){
        v.push_back(b);
    }
    void addCueBall(cueBall c){
        whiteBall=c;
    }
    int getBallCount(){
        return v.size();
    }
    bool inRange(Ball &b1, Ball &b2){
        Vector relativePos=Vector(b2.getX()-b1.getX(), b2.getY()-b1.getY());
        return (relativePos.modul()<b2.getR()+b1.getR());///TODO fa razele statice
    }
    void runShot(){
       int ctBalls=v.size();
       whiteBall.moveBall();
       for(int i=0; i<ctBalls; i++){///trebuie sa rulam testele separat pentru alba
          if(inRange(whiteBall, v[i])) whiteBall.collide(v[i]);//upcasting - am dat bila alba
       }
       ///acum, e nevoie sa facem operatiunile aferente si pentru alba, care e mai cu mot
       double whiteX=whiteBall.getX(), whiteY=whiteBall.getY(), rad=whiteBall.getR();
       if((whiteX-rad<0 && whiteY-rad<0) || (whiteX-rad<0 && whiteY+rad>l) ||
          (whiteX+rad>L && whiteY-rad<0) || (whiteX+rad>L && whiteY+rad>l)){
             whiteBall.potBall();
          }
        if(whiteX<rad || whiteY+rad>L)
            whiteBall.hitVertCushion();

        if(whiteY<rad || whiteY+rad>l)
            whiteBall.hitHorizCushion();
       for(int i=0; i<ctBalls; i++){///we move the balls one at a time
           v[i].moveBall();
           for(int j=0; j<ctBalls; j++){
              Vector dist=Vector(v[j].getX()-v[i].getX(), v[j].getY()-v[i].getY());
              if(dist.modul()<v[j].getR()+v[i].getR() && i!=j)
                 v[i].collide(v[j]);
           }
           double cx=v[i].getX(), cy=v[i].getY(), R=v[i].getR();///extract coordinates, to easily check cushion ricochet
           if( (cx-R<0 && cy-R<0) || (cx-R<0 && cy+R>l)
           ||  (cx+R>L && cy+R>l) || (cx+R>L && cy-R<0) ){ ///if the center is very close to a pocket, the ball is potted
              v[i].potBall();
           }
           if(cx<R||cx+R>L){///if it's too close to a vertical edge
             v[i].hitVertCushion();///ricochets sideways
             //std::cout<<"A ricosat din verticala\n";
           }
           if(cy<R||cy+R>l){///all the same, ricochets
             v[i].hitHorizCushion();
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
       std::cout<<"Bila alba la "<<whiteBall;
       for(int i=0; i<v.size(); i++){ ///nu or sa apara bile albe pe aici
          std::cout<<"Bila "<<v[i].getNo()<<" la "<<v[i];
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
int main(){///TODO see what speeds we should impart
    std::ifstream fin("balls.txt");
    int ballCount; double tableLength, tableWidth;
    cueBall c1;
    fin>>ballCount>>tableLength>>tableWidth;
    fin>>c1;
    Table t(tableLength, tableWidth, 0.0);///10 units long, 7 units wide -subject to change
    t.addCueBall(c1);///stiu ca e idiot, dar ei ii trebuie bila transferata altfel
    double topSpeed=0;
    for(int i=0; i<ballCount; i++){
        Ball b;
        fin>>b;
        t.addBall(b);
        topSpeed=std::max(topSpeed, b.getSpeed());///initial configuration of the balls
    }
    ///the actual game loop
    t.writeBalls();
    int shots=0;
    while(t.getBallCount()>0&&shots<5){
       std::cout<<"Trageti in bila alba, dati puterea pe x si pe y: \n";
       double vx, vy; std::cin>>vx>>vy;
       t.shot(vx, vy);///trebuie sa copiem toata bila, ceea ce e otara lent, dar ok
       topSpeed=t.getBall(0).getSpeed();
       while(topSpeed>0){
         t.runShot();
         topSpeed=0;
         for(int i=0; i<ballCount; i++){
            topSpeed=std::max(topSpeed, t.getBall(i).getSpeed());
         }
      }
      t.writeBalls();///ok, pare sa plimbe bila alba pe teren, acum sa punem si restul
      shots++;
    }
    Table t2(t);///quick 2-liner to test copy constructors
    t2.writeBalls();
    return 0;
}
