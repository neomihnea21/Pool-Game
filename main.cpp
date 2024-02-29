#include<iostream>
#include<vector>
#include<fstream>
#include<cmath>
#include<queue>
const double pi=3.14, dt=0.01, g=9.81;
inline double abs(double x){
    if(x<0) return -x;
    return x;
}
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
    double modul(){
        return sqrt(x*x+y*y);
    }
    double argument(){
        return atan2(y, x);
    }
    Vector operator+(Vector &other){
        Vector v2(x+other.x, y+other.y);
        return v2;
    }
    Vector operator-(Vector &other){
        Vector v2(x-other.x, y-other.y);
        return v2;
    }
};
class Ball{
    int no;
    double x, y, r, m, mu;///coordinates, radius, mass, sfc
    Vector v;
public:
    Ball(int no_=1, double x_=0, double y_=0, double r_=0, double m_=0, double mu_=0, Vector v_=0):
        no(no_), x(x_), y(y_), r(r_), m(m_), mu(mu_), v(v_) {}
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
    Vector getV(){ return v;}
    void moveBall(){
        double vx=v.getX(), vy=v.getY();
        x+=vx*dt, y+=vy*dt; ///nudge the ball in its direction of motion
        vx-=abs(mu*m*g*cos(v.argument()));///friction
        vy-=abs(mu*m*g*sin(v.argument()));
        if(vx<0) vx=0;
        if(vy<0) vy=0;///tiny check to keep ball from rollling backwards
        v=Vector(vx, vy);
    }
    void hitCushion(int tip){
        Vector newV;
        if(tip==0){///ricochets off top or bottom
           newV=Vector(v.getX(), -v.getY());
        }
        else if(tip==1){///ricochets off left or right
           newV=Vector(-v.getX(), v.getY());
        }
        v=newV;
    }
    void collide(Ball &other){
        return;///placeholder, TODO fully flesh out collision function
    }
};
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
    Ball getBall(int i){
        if(i<v.size())
            return v[i];///how do we deal with the warning
    }
    void addBall(Ball b){
        v.push_back(b);
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
           if(cx<R||cx+R>L)///if it's too close to a vertical edge
             v[i].hitCushion(1);///ricochets sideways
           if(cy<R||cy+R>l)///all the same, ricochets
             v[i].hitCushion(0);
           ///check if ball is potted
           if(cx-0.5*R<0 && cy-0.5*R<0)
              v.erase(v.begin()+i);///
       }
    }
};
int main(){
    std::ifstream fin("balls.txt");
    Table t(10, 7);///10 units long, 7 units wide -subject to change
    double topSpeed=0;
    for(int i=0; i<ballCount; i++){
        Ball b(1, 0.15, 0.15, 0.1, 0.15, 0.15, Vector(0.1, 0.1));
        t.addBall(b);
        topSpeed=max(topSpeed, b.speed());
    }
    while(topSpeed>0){
        t.runShot();
    }
    return 0;
}
