#include "mohism.h"

class Sandbox: public MH::Application
{
public:
    Sandbox(/* args */)
    {

    }
    ~Sandbox()
    {

    }
private:
    /* data */
};

MH::Application* MH::create_application()
{
    return new Sandbox();
}


