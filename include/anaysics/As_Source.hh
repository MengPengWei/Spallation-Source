// #ifndef AS_SOURCE_HH
// #define AS_SOURCE_HH

// // 设置在step,track,event,run等不同层面记录哪些信息（如粒子类型、能量、位置等），以及记录格式（如文本文件、ROOT文件等），设置好后，在各对应的action类中实现具体的记录逻辑（如在PrimaryGeneratorAction中记录初级粒子信息，在SteppingAction中记录每步的信息等）

// class As_Source
// {
// public:
//     As_Source() = default;
//     ~As_Source() = default;

//     G4Run* GetCurrentRun() const
//     {
//         return static_cast<G4Run*>(G4RunManager::GetRunManager()->GetCurrentRun());
//     }
//     G4Step*SetAnaysics_Step(G4Step* step)
//     {
//         // 在这里实现你想要的分析逻辑，比如记录粒子类型、能量、位置等信息
//         // 你可以使用 GetCurrentRun() 来获取当前的 run 对象，并在其中记录你需要的信息
//         return step;
//     }

// };



// #endif // AS_SOURCE_HH