#include "ActionInitialization.hh"
#include "EventAction.hh"
#include "RunAction.hh"
#include "SteppingAction.hh"
#include "TrackingAction.hh"
#include "PrimaryGeneratorAction.hh"

ActionInitialization::ActionInitialization(DetectorConstruction* detector)
    : G4VUserActionInitialization(),
      fDetector(detector)
{
}

void ActionInitialization::BuildForMaster() const//主线程构建,主要功能为设置RunAction，用于统计整个模拟过程的总结果
{
    RunAction* runAction = new RunAction();
    SetUserAction(runAction);
}

void ActionInitialization::Build() const//工作线程构建,设置PrimaryGeneratorAction、RunAction、EventAction、SteppingAction和TrackingAction等用户自定义的动作类
{
    SetUserAction(new PrimaryGeneratorAction());

    RunAction* runAction = new RunAction();
    SetUserAction(runAction);

    EventAction* eventAction = new EventAction(runAction);
    SetUserAction(eventAction);

  
    TrackingAction* trackingAction = new TrackingAction(fDetector);//创建一个TrackingAction对象，用于处理跟踪相关的任务。
    SetUserAction(trackingAction);
    
    SteppingAction* steppingAction = new SteppingAction(fDetector, eventAction);//创建一个SteppingAction对象，用于处理步进相关的任务。
    SetUserAction(steppingAction);
}
