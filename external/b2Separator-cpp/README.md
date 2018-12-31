

# [b2Separator-cpp](https://github.com/delorenj/b2Separator-cpp)

Create non-convex, complex shapes with Box2D. Ported from Antoan Angelov's b2Separator class.

* Source: [https://github.com/delorenj/b2Separator-cpp](https://github.com/delorenj/b2Separator-cpp)

## Example Use

        b2Body *body;
        b2BodyDef *bodyDef = new b2BodyDef();
        b2FixtureDef *fixtureDef = new b2FixtureDef();
        b2Separator* sep = new b2Separator();

        bodyDef->type = b2_dynamicBody;
        bodyDef->position.Set(p.x/PTM_RATIO, p.y/PTM_RATIO);
        body = getWorld()->CreateBody(bodyDef);
        fixtureDef->restitution = 0.4f;
        fixtureDef->friction = 0.2f;
        fixtureDef->density = 4;
        
        vector<b2Vec2>* vec = new vector<b2Vec2>();
        vec->push_back(b2Vec2(-3, -3));
        vec->push_back(b2Vec2(3, -3));
        vec->push_back(b2Vec2(3, 0));
        vec->push_back(b2Vec2(0, 0));
        vec->push_back(b2Vec2(-3, 3));

        if(sep->Validate(*vec)==0)
        {
            CCLog("Yay! Those vertices are good to go!");
        }
        else
        {
            CCLog("Oh, I guess you effed something up :(");
        }
    
        sep->Separate(body, fixtureDef, vec, PTM_RATIO);

