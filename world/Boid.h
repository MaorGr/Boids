#include "../external/quad/Quadtree.h"
#include <SFML/Graphics.hpp> 
// todo(maor): remove SFML dependency, switch to standard 2d lib, handle visualization separately 

class Boid {
private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    float touchRadius;
    float sensingRadius;

public:
    Boid(float x, float y, float vx, float vy);

    Boid(float x, float y);

    float getX() const;
    void setX(float x);    
    float getY() const;
    void setY(float y);
    quadtree::Box <float> box;
    quadtree::Box <float> getBox() const;
    
    template <typename GetBoxFunc>
    std::vector<Boid> getNeighbors(float radius, const quadtree::Quadtree<Boid*, GetBoxFunc>& quadtree) const;

    void updateAvoidanceDirection(const std::vector<Boid>& neighbors);

    // template <typename GetBoxFunc>
    void update(const quadtree::Quadtree<Boid*, GetBoxFunctor>& quadtree);
    std::optional<std::pair<sf::Vector2f, sf::Vector2f>> getBoidsAvgInfo(const std::vector<Boid>& boids);

};