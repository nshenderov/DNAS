/*******************************************************************************
*
* FILENAME : dispather_test.cpp
* 
* AUTHOR : Nick Shenderov
*
* DATE : 27.09.2023
* 
*******************************************************************************/

#include <iostream> // std::cout, std::endl

#include "testing.hpp" // nsrd::testing::Testscmp, nsrd::testing::UnitTest
#include "dispatcher.hpp" // nsrd::Dispatcher

using namespace nsrd;
using namespace nsrd::testing;

namespace
{
void TestGeneral();
void TestBroadcastToOne();
void TestBroadcastToTwo();
void TestBroadcastToThree();
void TestRemOneDurBroad();
void TestRemAllDurBroad();
void TestAddOneDurBroad();
void TestAddTwoDurBroad();
} // namespace

int main()
{
    Testscmp cmp;
    cmp.AddTest(UnitTest("General", TestGeneral));
    cmp.AddTest(UnitTest("BroadcastToOne", TestBroadcastToOne));
    cmp.AddTest(UnitTest("BroadcastToTwo", TestBroadcastToTwo));
    cmp.AddTest(UnitTest("BroadcastToThree", TestBroadcastToThree));
    cmp.AddTest(UnitTest("RemOneDurBroad", TestRemOneDurBroad));
    cmp.AddTest(UnitTest("RemAllDurBroad", TestRemAllDurBroad));
    cmp.AddTest(UnitTest("AddOneDurBroad", TestAddOneDurBroad));
    cmp.AddTest(UnitTest("AddTwoDurBroad", TestAddTwoDurBroad));
    cmp.Run();

    return (0);
}
namespace
{

class ViewEvent;
class ModelEvent;

/* ControlWindow */
/* ************************************************************************** */

class ControlWindow
{
public:
    ControlWindow() = default;
    ~ControlWindow();

    ControlWindow(ControlWindow &other) = delete;
    ControlWindow(ControlWindow &&other) = delete;
    ControlWindow &operator=(ControlWindow &other) = delete;
    ControlWindow &operator=(ControlWindow &&other) = delete;

    void BroadCastToModel(ModelEvent &event);
    void SubcsribeToModel(Callback<ModelEvent> *sub);

    void BroadCastToView(ViewEvent &event);
    void SubcsribeToView(Callback<ViewEvent> *sub);

private:
    Dispatcher<ModelEvent> m_model_dispatcher;
    Dispatcher<ViewEvent> m_view_dispatcher;
};


/* ViewWindow */
/* ************************************************************************** */

class ViewWindow : public Callback<ModelEvent>
{
public:
    explicit ViewWindow(int number = 0);
    ~ViewWindow(){};

    ViewWindow(ViewWindow &other) = delete;
    ViewWindow(ViewWindow &&other) = delete;
    ViewWindow &operator=(ViewWindow &other) = delete;
    ViewWindow &operator=(ViewWindow &&other) = delete;

    void AddController(ControlWindow *controller);
    void AddViewToSucribe(ViewWindow *view_to_subscribe);
    void RequestData();
    int GetValue();

private:
    void Notify(ModelEvent event);
    void NotifyOnDeath(){ m_some_value = -1; };

    ControlWindow *m_controller;
    std::list<ViewWindow *> m_views_to_subscribe;
    int m_some_value = 0;
    int m_number = 0;
};

/* DataModel */
/* ************************************************************************** */

class DataModel : public Callback<ViewEvent>
{
public:
    explicit DataModel(int data = 0): m_data(data) {};
    ~DataModel(){};

    DataModel(DataModel &other) = delete;
    DataModel(DataModel &&other) = delete;
    DataModel &operator=(DataModel &other) = delete;
    DataModel &operator=(DataModel &&other) = delete;

    void AddController(ControlWindow *controller);
    int GetData();
    void SetData(int data);

private:
    void Notify(ViewEvent event);
    void NotifyOnDeath(){ m_data = -1; };

    ControlWindow *m_controller;
    int m_data;
};

/* Events */
/* ************************************************************************** */

class ViewEvent
{
public:
private:
};

class ModelEvent
{
public:
    explicit ModelEvent(int data);
    int GetData();

private:
    int m_data;
};

ModelEvent::ModelEvent(int data)
    : m_data(data)
{}

int ModelEvent::GetData()
{
    return (m_data);
}

/* ControlWindow */
/* ************************************************************************** */

ControlWindow::~ControlWindow()
{}

void ControlWindow::BroadCastToModel(ModelEvent &event)
{
    m_model_dispatcher.Broadcast(event);
}

void ControlWindow::SubcsribeToModel(Callback<ModelEvent> *sub)
{
    m_model_dispatcher.Subscribe(sub);
}

void ControlWindow::BroadCastToView(ViewEvent &event)
{
    m_view_dispatcher.Broadcast(event);
}

void ControlWindow::SubcsribeToView(Callback<ViewEvent> *sub)
{
    m_view_dispatcher.Subscribe(sub);
}

/* ViewWindow */
/* ************************************************************************** */

ViewWindow::ViewWindow(int number)
    : m_controller(0), m_views_to_subscribe(), m_some_value(0), m_number(number)
{}

void ViewWindow::Notify(ModelEvent event)
{
    m_some_value += event.GetData();

    if (666 == m_some_value)
    {
        DispatcherUnsubscribe();
        m_some_value = -1;
    }
    else if (555 == m_some_value)
    {
        for (auto it : m_views_to_subscribe)
        {
            DispatcherSubscribe(it);
        }   
     
        m_some_value = 555;
    }
    else if (m_number == m_some_value)
    {
        DispatcherUnsubscribe();
        m_some_value = -1;
    }
}

void ViewWindow::RequestData()
{
    ViewEvent e;
    m_controller->BroadCastToView(e);
}

void ViewWindow::AddViewToSucribe(ViewWindow *view_to_subscribe)
{
    m_views_to_subscribe.push_back(view_to_subscribe);
}

int ViewWindow::GetValue()
{
    return (m_some_value);
}

void ViewWindow::AddController(ControlWindow *controller)
{
    m_controller = controller;
}

/* DataModel */
/* ************************************************************************** */

void DataModel::Notify(ViewEvent event)
{
    ModelEvent e(m_data);
    m_controller->BroadCastToModel(e);

    (void) event;
}

void DataModel::AddController(ControlWindow *controller)
{
    m_controller = controller;
}

int DataModel::GetData()
{
    return (m_data);
}

void DataModel::SetData(int data)
{
    m_data = data;
}


/* Tests */
/* ************************************************************************** */

void TestGeneral()
{
    ControlWindow *controller = new ControlWindow();

    ViewWindow view;
    DataModel model(15);
    view.AddController(controller);
    model.AddController(controller);

    controller->SubcsribeToView(&model);
    controller->SubcsribeToModel(&view);
    
    view.RequestData();
    TH_ASSERT(15 == view.GetValue());

    delete controller;

    TH_ASSERT(-1 == view.GetValue());
    TH_ASSERT(-1 == model.GetData());
}

void TestBroadcastToOne()
{
    ControlWindow controller;

    ViewWindow view;
    DataModel model(15);

    view.AddController(&controller);
    model.AddController(&controller);

    controller.SubcsribeToModel(&view);
    controller.SubcsribeToView(&model);

    view.RequestData();

    TH_ASSERT(15 == view.GetValue());
}

void TestBroadcastToTwo()
{
    ControlWindow controller;

    ViewWindow view1;
    ViewWindow view2;
    DataModel model(15);

    view1.AddController(&controller);
    view2.AddController(&controller);
    model.AddController(&controller);

    controller.SubcsribeToModel(&view1);
    controller.SubcsribeToModel(&view2);
    controller.SubcsribeToView(&model);

    view2.RequestData();

    TH_ASSERT(15 == view1.GetValue());
    TH_ASSERT(15 == view2.GetValue());
}

void TestBroadcastToThree()
{
    ControlWindow controller;

    ViewWindow view1;
    ViewWindow view2;
    ViewWindow view3;
    DataModel model(15);

    view1.AddController(&controller);
    view2.AddController(&controller);
    view3.AddController(&controller);
    model.AddController(&controller);

    controller.SubcsribeToModel(&view1);
    controller.SubcsribeToModel(&view2);
    controller.SubcsribeToModel(&view3);
    controller.SubcsribeToView(&model);

    view3.RequestData();

    TH_ASSERT(15 == view1.GetValue());
    TH_ASSERT(15 == view2.GetValue());
    TH_ASSERT(15 == view3.GetValue());
}

void TestRemOneDurBroad()
{
    ControlWindow controller;

    ViewWindow view1;
    ViewWindow view2(2);
    ViewWindow view3;
    DataModel model(2);

    view1.AddController(&controller);
    view2.AddController(&controller);
    view3.AddController(&controller);
    model.AddController(&controller);

    controller.SubcsribeToModel(&view1);
    controller.SubcsribeToModel(&view2);
    controller.SubcsribeToModel(&view3);
    controller.SubcsribeToView(&model);

    view3.RequestData();

    TH_ASSERT(2 == view1.GetValue());
    TH_ASSERT(-1 == view2.GetValue());
    TH_ASSERT(2 == view3.GetValue());
}

void TestRemAllDurBroad()
{
    ControlWindow controller;

    ViewWindow view1;
    ViewWindow view2;
    ViewWindow view3;
    DataModel model(666);

    view1.AddController(&controller);
    view2.AddController(&controller);
    view3.AddController(&controller);
    model.AddController(&controller);

    controller.SubcsribeToModel(&view1);
    controller.SubcsribeToModel(&view2);
    controller.SubcsribeToModel(&view3);
    controller.SubcsribeToView(&model);

    view3.RequestData();

    TH_ASSERT(-1 == view1.GetValue());
    TH_ASSERT(-1 == view2.GetValue());
    TH_ASSERT(-1 == view3.GetValue());
}

void TestAddOneDurBroad()
{
    ControlWindow controller;
    ViewWindow view_to_subscribe;

    ViewWindow view;
    DataModel model(555);

    view.AddController(&controller);
    view.AddViewToSucribe(&view_to_subscribe);

    model.AddController(&controller);

    controller.SubcsribeToModel(&view);
    controller.SubcsribeToView(&model);

    view.RequestData();

    TH_ASSERT(555 == view.GetValue());
    TH_ASSERT(0 == view_to_subscribe.GetValue());

    model.SetData(15);
    
    view.RequestData();

    TH_ASSERT(570 == view.GetValue());
    TH_ASSERT(15 == view_to_subscribe.GetValue());
}

void TestAddTwoDurBroad()
{
    ControlWindow controller;
    ViewWindow view_to_subscribe1;
    ViewWindow view_to_subscribe2;
    ViewWindow view_to_subscribe3;

    ViewWindow view1;
    ViewWindow view2;
    ViewWindow view3;
    DataModel model(555);

    view1.AddController(&controller);
    view2.AddController(&controller);
    view3.AddController(&controller);
    view1.AddViewToSucribe(&view_to_subscribe1);
    view1.AddViewToSucribe(&view_to_subscribe2);
    view1.AddViewToSucribe(&view_to_subscribe3);

    model.AddController(&controller);

    controller.SubcsribeToModel(&view1);
    controller.SubcsribeToModel(&view2);
    controller.SubcsribeToModel(&view3);
    controller.SubcsribeToView(&model);

    view1.RequestData();

    TH_ASSERT(555 == view1.GetValue());
    TH_ASSERT(555 == view2.GetValue());
    TH_ASSERT(555 == view3.GetValue());
    TH_ASSERT(0 == view_to_subscribe1.GetValue());
    TH_ASSERT(0 == view_to_subscribe2.GetValue());
    TH_ASSERT(0 == view_to_subscribe3.GetValue());

    model.SetData(15);
    
    view1.RequestData();

    TH_ASSERT(570 == view1.GetValue());
    TH_ASSERT(570 == view2.GetValue());
    TH_ASSERT(570 == view3.GetValue());
    TH_ASSERT(15 == view_to_subscribe1.GetValue());
    TH_ASSERT(15 == view_to_subscribe2.GetValue());
    TH_ASSERT(15 == view_to_subscribe3.GetValue());
}

} // namespace