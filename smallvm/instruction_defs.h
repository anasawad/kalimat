// VM_INSTRUCTION(OpCode, TakesStack, PutsStack)
VM_INSTRUCTION(PushVal,         0, 1)
VM_INSTRUCTION(PushLocal,       0, 1)
VM_INSTRUCTION(PopLocal,        1, 0)
VM_INSTRUCTION(PushGlobal,      0, 1)
VM_INSTRUCTION(PopGlobal,       1, 0)
VM_INSTRUCTION(PushNull,        0, 1)
VM_INSTRUCTION(GetRef,          1, 1)
VM_INSTRUCTION(SetRef,          2, 0)
VM_INSTRUCTION(Add,             2, 1)
VM_INSTRUCTION(Sub,             2, 1)
VM_INSTRUCTION(Mul,             2, 1)
VM_INSTRUCTION(Div,             2, 1)
VM_INSTRUCTION(And,             2, 1)
VM_INSTRUCTION(Or,              2, 1)
VM_INSTRUCTION(Not,             1, 1)
VM_INSTRUCTION(Jmp,             0, 0)
VM_INSTRUCTION(JmpVal,          1, 0)
VM_INSTRUCTION(If,              1, 0)
VM_INSTRUCTION(Lt,              2, 1)
VM_INSTRUCTION(Gt,              2, 1)
VM_INSTRUCTION(Eq,              2, 1)
VM_INSTRUCTION(Ne,              2, 1)
VM_INSTRUCTION(Le,              2, 1)
VM_INSTRUCTION(Ge,              2, 1)
VM_INSTRUCTION(Tail,            0, 0)
VM_INSTRUCTION(Call,            0, 0)
VM_INSTRUCTION(CallMethod,      0, 0)
VM_INSTRUCTION(CallRef,         0, 0)
VM_INSTRUCTION(Ret,             0, 0)
VM_INSTRUCTION(Apply,           2, 0)
VM_INSTRUCTION(CallExternal,    0, 0)
VM_INSTRUCTION(Nop,             0, 0)
VM_INSTRUCTION(SetField,        2, 0)
VM_INSTRUCTION(GetField,        1, 1)
VM_INSTRUCTION(GetFieldRef,     1, 1)
VM_INSTRUCTION(GetArr,          2, 1)
VM_INSTRUCTION(SetArr,          3, 0)
VM_INSTRUCTION(GetArrRef,       2, 1)
VM_INSTRUCTION(New,             0, 1)
VM_INSTRUCTION(NewArr,          1, 1)
VM_INSTRUCTION(ArrLength,       1, 1)
VM_INSTRUCTION(NewMD_Arr,       1, 1)
VM_INSTRUCTION(GetMD_Arr,       2, 1)
VM_INSTRUCTION(SetMD_Arr,       3, 1)
VM_INSTRUCTION(GetMD_ArrRef,    2, 1)
VM_INSTRUCTION(MD_ArrDimensions,1, 1)
VM_INSTRUCTION(PushConstant,    0, 1)
VM_INSTRUCTION(Neg,             1, 1)
VM_INSTRUCTION(RegisterEvent,   0, 0)
VM_INSTRUCTION(Isa,             1, 1)
VM_INSTRUCTION(Send,            2, 0)
VM_INSTRUCTION(Receive,         1, 1)
VM_INSTRUCTION(Select,          2, 2) // actuall 2, (1 or 2)
VM_INSTRUCTION(Break,           0, 0)
VM_INSTRUCTION(Tick,            0, 1)
VM_INSTRUCTION(Checktimeslice,  0, 0)
VM_INSTRUCTION(JmpIfEq,         2, 0)
VM_INSTRUCTION(JmpIfNe,         2, 0)
VM_INSTRUCTION(Increment,       1, 1)
VM_INSTRUCTION(Decrement,       1, 1)
VM_INSTRUCTION(NormalCallCached,0, 0)
VM_INSTRUCTION(TailCallCached,  0, 0)
VM_INSTRUCTION(LaunchCallCached,0, 0)
VM_INSTRUCTION(PushLocalCached, 0, 1)