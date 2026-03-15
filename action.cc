#include "action.hh"

MyActionInitialization::MyActionInitialization()
{}

MyActionInitialization::~MyActionInitialization()
{}

void MyActionInitialization::BuildForMaster() const
{
    // Le RunAction du master gère l'écriture finale du fichier ROOT
    SetUserAction(new MyRunAction());
}

void MyActionInitialization::Build() const
{
    // Chaque worker a ses propres instances (thread-local)
    MyRunAction   *runAction = new MyRunAction();
    MyEventAction *evtAction = new MyEventAction(runAction);

    SetUserAction(new MyPrimaryGenerator());
    SetUserAction(runAction);
    SetUserAction(evtAction);
}
