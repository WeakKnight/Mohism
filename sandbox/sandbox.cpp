#include "mohism.h"

class TestLayer: public MH::Layer
{
    public:
        TestLayer()
        : Layer("Test")
        {
        }

        void on_update() override
        {
            LOG_INFO("Test Layer Updated");
        }

        void on_event(MH::Event& event) override
        {
            LOG_INFO("{0}", event.to_string());
        }
};

class Sandbox: public MH::Application
{
public:
    Sandbox(/* args */)
    {
        push_layer(new TestLayer());
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


