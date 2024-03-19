#include<iostream>
#include<vector>
#include<fstream>
#include<cmath>
#include<queue>

std::ifstream fin("balls.txt");
/*inline double abs(double x){
    if(x<0) return -x;
    return x;
}*/
class Vector{
    double x, y;
public:
    Vector(double x_=0, double y_=0): x(x_), y(y_) {}
    double getX() const{
        return x;
    }
    double getY() const{
        return y;
    }
    void setX(double x_) {x=x_;}
    void setY(double y_) {y=y_;}///for some reason, we need setters
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
    friend std::ostream& operator <<(std::ostream &out, Vector &v);
    friend std::istream& operator >>(std::istream &in, Vector &v);
    ///for some reason, the vector's custom = fucks with everything
};
std::ostream& operator<<(std::ostream &out, Vector v){
    out<<v.getX()<<" "<<v.getY()<<" ";
    return out;
}
std::istream& operator >>(std::istream &in, Vector &v){
    in>>v.x>>v.y;
    return in;
}
class Ball{
    int no;
    double x, y, r, m, mu;///coordinates, radius, mass, sfc
    Vector v;
    double pi=3.14, dt=0.01, g=9.81, EPS=0.5; ///nu sunt statice, ca mai sunt probleme
public:
    Ball(int no_=1, double x_=0, double y_=0, double r_=0, double m_=0, double mu_=0, Vector v_=0):
        no(no_), x(x_), y(y_), r(r_), m(m_), mu(mu_), v(v_) {}
    void read(){ ///can't overload operator
         fin>>no>>x>>y>>r>>m>>mu>>v;
    }
    void shootBall(int vx, int vy){///todo asta sa apara doar in clasa derivata cueBall, singura in care se poate da
        Vector copie(vx, vy);
        v=copie;
    }
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
        vx-=std::abs(mu*g*cos(v.argument()))*dt;///friction
        vy-=std::abs(mu*g*sin(v.argument()))*dt;
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
    friend std::ostream& operator<<(std::ostream &out, Ball b);
};
std::ostream& operator<<(std::ostream& out, Ball b){///scrie bila
    out<<b.x<<" "<<b.y<<"\n";
    return out;
}
class Table{
    double L, l, pocketSize;
    int balls;///count the balls on the table
    std::vector<Ball> v;
public:
    Table(double length=0, double width=0, double pocketSize=0, int balls_=0, std::vector<Ball>vt={}){
       L=length, l=width;
       v=vt; balls=balls_;
    }
    std::pair<double, double> getSize(){
        return std::make_pair(L, l);
    }
    void shot(int i, int vx, int vy){
        v[i].shootBall(vx, vy);
    }
    Ball getBall(int i){
            return v[i];///how do we deal with asking for nonexistent balls
    }
    void addBall(Ball b){
        v.push_back(b);
    }
    int getBallCount(){
        return v.size();
    }
    void runShot(){
       int ctBalls=v.size();
       for(int i=0; i<ctBalls; i++){///we move the balls one at a time
           v[i].moveBall();
           for(int j=0; j<ctBalls; j++){
              Vector dist=Vector(v[j].getX()-v[i].getX(), v[j].getY()-v[i].getY());
              if(dist.modul()<v[j].getR()+v[i].getR() && i!=j)
                 v[i].collide(v[j]);
           }
           double cx=v[i].getX(), cy=v[i].getY(), R=v[i].getR();///extract coordinates, to easily check cushion ricochet
           if(cx<R||cx+R>L){///if it's too close to a vertical edge
             v[i].hitVertCushion();///ricochets sideways
             std::cout<<"A ricosat din verticala\n";
           }
           if(cy<R||cy+R>l){///all the same, ricochets
             v[i].hitHorizCushion();
             std::cout<<"A ricosat din orizontala\n";
           }
           ///check if ball is potted
           if( (cx-0.5*R<0 && cy-0.5*R<0) || (cx-0.5*R<0 && cy+0.5*R>l)
           ||  (cx+0.5*R>L && cy+0.5*R>l) || (cx+0.5*R>L && cy-0.5*R<0) ){ ///if the center is very close to a pocket, the ball is potted
              if(v[i].getNo()==0){///if we pot the cue ball
                std::cout<<"Ai bagat alba, se repune de la centru\n";
                Ball newCueBall(0, L/2, l/2, 0.15, 0.1, 0.15, Vector(0, 0));///replace the cue ball mid-board
                v[0]=newCueBall;///and move it over
              }
              else{
                 std::cout<<"Ai bagat bila "<<i<<" \n";
              }
           }
       }
    }
    ///rule of three for the tables
    Table(const Table &other): L(other.L), l(other.l), pocketSize(other.pocketSize), v(other.v), balls(v.size()){ }

    Table operator=(Table &other){
        Table t2(other);
        return *this;
    }
    ~Table(){
        L=0, l=0, balls=0, pocketSize=0;
        v.clear();
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
void writeBalls(Table t){
    std::cout<<"Avem bilele urmatoare:\n";
    for(int i=0; i<t.getBallCount(); i++){
      if(t.getBall(i).getNo()==0)///cue ball is always 0
        std::cout<<"Bila alba la "<<t.getBall(i);
      else
        std::cout<<"Bila "<<i<<" la "<<t.getBall(i);
    }
}
int main(){///TODO see what speeds we should impart
    Table t(10, 7);///10 units long, 7 units wide -subject to change
    double topSpeed=0;
    int ballCount; fin>>ballCount;
    for(int i=0; i<ballCount; i++){
        Ball b;
        b.read();
        t.addBall(b);
        topSpeed=std::max(topSpeed, b.getSpeed());///initial configuration of the balls
    }
    ///the actual game loop
    writeBalls(t);
    int shots=0;
    while(t.getBallCount()>0&&shots<5){
       std::cout<<"Trageti in bila alba, dati puterea pe x si pe y: \n";
       double vx, vy; std::cin>>vx>>vy;
       t.shot(0, vx, vy);///trebuie ssa copiem toata bila, ceea ce e otara lent, dar ok
       topSpeed=t.getBall(0).getSpeed();
       while(topSpeed>0){
         t.runShot();
         topSpeed=0;
         for(int i=0; i<ballCount; i++){
            topSpeed=std::max(topSpeed, t.getBall(i).getSpeed());
         }
      }
      writeBalls(t);///ok, pare sa plimbe bila alba pe teren, acum sa punem si restul
      shots++;
    }
    Table t2(t);///quick 2-liner to test copy constructors
    writeBalls(t2);
    return 0;
}
