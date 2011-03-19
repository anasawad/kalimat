/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include <QVector>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QStack>

#include "codegenerator_incl.h"
#include "../utils.h"
template<typename T1, typename T2> bool isa(T2 *value)
{
    T1 *test = dynamic_cast<T1 *>(value);
    return test!=NULL;
}

CodeGenerator::CodeGenerator()
{
    Init();
}

void CodeGenerator::Init()
{
    _asm.Init();
    codePosKeyCount = 0;
    currentModuleName = "";
    currentCodeDoc = NULL;
}
QString CodeGenerator::getOutput()
{
    return _asm.getOutput();
}

void CodeGenerator::defineInCurrentScope(QString var)
{
    if(!scopeStack.empty())
        scopeStack.top().bindings.insert(var);
}

void CodeGenerator::generate(Program *program, CodeDocument *curDoc)
{
    QVector<Declaration *> declarations;
    QVector<Statement *> statements;
    currentCodeDoc = curDoc;
/*
    for(int i=0; i<program->usedModuleCount(); i++)
    {
        loadModule(program->usedModule(i));
    }
    */
    for(int i=0; i<program->elementCount(); i++)
    {
        TopLevel *tl = program->element(i);
        if(isa<Declaration>(tl))
            declarations.append(dynamic_cast<Declaration *>(tl));
        else if(isa<Statement>(tl))
            statements.append(dynamic_cast<Statement *>(tl));
        else
            throw CompilerException(tl, ASTMustBeStatementOrDeclaration).arg(tl->toString());
    }

    for(int i=0; i<declarations.count(); i++)
    {
        firstPass(declarations[i]);
    }
    for(int i=0; i<declarations.count(); i++)
    {
        secondPass(declarations[i]);
    }
    for(int i=0; i<declarations.count(); i++)
    {
        thirdPass(declarations[i]);
    }
    checkInheritanceCycles();
    for(int i=0; i<declarations.count(); i++)
    {
        generateDeclaration(declarations[i]);
    }
}

void CodeGenerator::compileModule(Module *module, CodeDocument *curDoc)
{
    // Copy-pasted from generate(Program *)
    currentCodeDoc = curDoc;
    QVector<Declaration *> declarations;

 /*   for(int i=0; i<module->usedModuleCount(); i++)
    {
        loadModule(module->usedModule(i));
    }
    */
    for(int i=0; i<module->declCount(); i++)
    {
        Declaration *decl = module->decl(i);
        declarations.append(dynamic_cast<Declaration *>(decl));
    }

    for(int i=0; i<declarations.count(); i++)
    {
        firstPass(declarations[i]);
    }
    for(int i=0; i<declarations.count(); i++)
    {
        secondPass(declarations[i]);
    }
    for(int i=0; i<declarations.count(); i++)
    {
        thirdPass(declarations[i]);
    }
    checkInheritanceCycles();
    for(int i=0; i<declarations.count(); i++)
    {
        generateDeclaration(declarations[i]);
    }
}

void CodeGenerator::firstPass(Declaration * decl)
{
    if(isa<ClassDecl>(decl))
    {
        ClassDecl *cd = (ClassDecl *) decl;
        QString name = cd->name()->name;
        if(allClasses.contains(name))
            throw CompilerException(decl, ClassAlreadyExists).arg(name);
        else
            allClasses[name] = cd;
        return;
    }

    if(isa<ProcedureDecl>(decl))
    {
        ProcedureDecl *proc = (ProcedureDecl *) decl;
        allProcedures[proc->procName()->name] = proc;
    }
    if(isa<FunctionDecl>(decl))
    {
        FunctionDecl *func = (FunctionDecl *) decl;
        allFunctions[func->procName()->name] = func;
    }
    if(isa<GlobalDecl>(decl))
    {
        generateGlobalDeclaration(dynamic_cast<GlobalDecl *>(decl));
    }

}
void CodeGenerator::secondPass(Declaration * decl)
{
    if(isa<ClassDecl>(decl))
    {
        ClassDecl *cd = (ClassDecl *) decl;
        if(cd->ancestor() !=NULL)
        {
            QString ancestorName = cd->ancestor()->name;
            if(!allClasses.contains(ancestorName))
            {
                throw CompilerException(decl, AncestorClassXforClassYdoesntExist)
                                            .arg(ancestorName).arg(cd->name()->name);
            }
            ClassDecl *ancestor = allClasses[ancestorName];
            cd->setAncestorClass(QSharedPointer<ClassDecl>(ancestor));
        }
    }
}
void CodeGenerator::thirdPass(Declaration * decl)
{
    if(isa<MethodDecl>(decl))
    {
        MethodDecl *md = (MethodDecl *) decl;
        QString name = md->procName()->name;
        QString className = md->className()->name;
        if(!allClasses.contains(className))
            throw CompilerException(decl, MethodDefinedForNotYetExistingClass).arg(className);
        else
        {
            ClassDecl *theClass = allClasses[className];
            if(theClass->containsMethod(name))
                throw CompilerException(decl, MethodCalledXwasAlreadyDefinedForClassY).arg(name).arg(className);
            if(!theClass->containsPrototype(name))
                throw CompilerException(decl, MethodXwasNotDeclaredInClassY).arg(name).arg(className);

            // We subtract the 1 to account for the extra 'this' parameter in md
            if(theClass->methodPrototype(name).arity != md->formalCount()-1)
                throw CompilerException(decl, MethodXwasDeclaredWithDifferentArityInClassY).arg(name).arg(className);
            if(theClass->methodPrototype(name).isFunction && !md->isFunctionNotProcedure)
                throw CompilerException(decl, MethodXwasDeclaredAfunctionButImplementedAsProcedureInClassY).arg(name).arg(className);
            if(!theClass->methodPrototype(name).isFunction && md->isFunctionNotProcedure)
                throw CompilerException(decl, MethodXwasDeclaredAprocedureButImplementedAsFunctionInClassY).arg(name).arg(className);
            allClasses[className]->insertMethod(name, QSharedPointer<MethodDecl>(md));
        }
        return;
    }
}

void CodeGenerator::checkInheritanceCycles()
{
    //todo:
}

void CodeGenerator::generateDeclaration(Declaration * decl)
{
    if(isa<ProceduralDecl>(decl))
    {
        ProceduralDecl *pd = (ProceduralDecl *) decl;
        pushProcedureScope(pd);
    }

    if(isa<ProcedureDecl>(decl))
    {
        ProcedureDecl *pd = (ProcedureDecl *) decl;
        if(pd->procName()->name == "%main")
            generateEntryPoint(pd->body()->getStatements());
        else
            generateProcedureDeclaration(dynamic_cast<ProcedureDecl *>(decl));
    }
    else if(isa<FunctionDecl>(decl))
    {
        generateFunctionDeclaration(dynamic_cast<FunctionDecl *>(decl));
    }
    else if(isa<ClassDecl>(decl))
    {
        generateClassDeclaration(dynamic_cast<ClassDecl *>(decl));
    }
    else if(isa<MethodDecl>(decl))
    {
        // It'll be handled by it's owning class, the thirdPass()
        // introduced them to each other. But we need this 'else' clause
        // to stop the final 'else' from throwing a "supported yet" exception
    }
    else if(isa<GlobalDecl>(decl))
    {
        // It's handled in the firstPass().
        // But we need this 'else' clause to stop the final 'else' from throwing a "not supported yet" exception
    }
    else
    {
        throw CompilerException(decl, DeclarationNotSupported).arg(decl->toString());
    }

    if(isa<ProceduralDecl>(decl))
    {
        popProcedureScope();
    }
}

void CodeGenerator::pushProcedureScope(ProceduralDecl *pd)
{
    Context c;
    c.proc = pd;
    scopeStack.push(c);
    for(int i=0; i<pd->formalCount(); i++)
        defineInCurrentScope(pd->formal(i)->name);
}
void CodeGenerator::popProcedureScope()
{
    scopeStack.pop();
}

void CodeGenerator::generateProcedureDeclaration(ProcedureDecl * decl)
{
    gen(decl, QString(".method %1 %2 0").arg(decl->procName()->name).arg(decl->formalCount()));
    for(int i=0; i<decl->formalCount(); i++)
    {
        gen(decl, "popl " + decl->formal(i)->name);
    }
    generateStatement(decl->body());
    gen(decl, "ret");
    gen(decl,".endmethod");
}

void CodeGenerator::generateFunctionDeclaration(FunctionDecl * decl)
{
    gen(decl, QString(".method %1 %2 1").arg(decl->procName()->name).arg(decl->formalCount()));
    for(int i=0; i<decl->formalCount(); i++)
    {
        gen(decl, "popl " + decl->formal(i)->name);
    }

    generateStatement(decl->body());
    gen(decl, "ret");
    gen(decl,".endmethod");
}

void CodeGenerator::generateClassDeclaration(ClassDecl *decl)
{
    gen(decl, ".class "+decl->name()->name);
    if(decl->ancestor() != NULL)
    {
        QString parent = decl->ancestor()->name;
        gen(decl->ancestor(), ".extends "+ parent);
    }
   /* if(decl->prototypeCount() > decl->methodCount())
        throw CompilerException(decl, QString("Class '%1' has some unimplemented methods").arg(decl->name()->name));
        */
    for(int i=0;i<decl->fieldCount();i++)
    {
        gen(decl,".field " + decl->field(i)->name);
    }
    for(int i=0; i<decl->methodCount(); i++)
    {
        MethodDecl *method = decl->method(i);
        pushProcedureScope(method);
        generateMethodDeclaration(method);
        popProcedureScope();
    }
    gen(decl,".endclass");
}
void CodeGenerator::generateGlobalDeclaration(GlobalDecl *decl)
{
    this->declaredGlobalVariables.insert(decl->varName);
}

void CodeGenerator::generateMethodDeclaration(MethodDecl *decl)
{
    QString name = decl->procName()->name;
    int numRet = decl->isFunctionNotProcedure? 1: 0;
    gen(decl, QString(".method %1 %2 %3").arg(name).arg(decl->formalCount()).arg(numRet));
    for(int i=0; i<decl->formalCount(); i++)
    {
        gen(decl, "popl " + decl->formal(i)->name);
    }
    generateStatement(decl->body());
    gen(decl, "ret");
    gen(decl,".endmethod");
}

void CodeGenerator::generateEntryPoint(QVector<Statement *> statements)
{
    gen(".method main");
    for(int i=0; i<statements.count(); i++)
    {
        generateStatement(statements[i]);
    }
    gen("pushv 0");
    //gen("ret");
    gen(".endmethod");
}
void CodeGenerator::generateStatement(Statement *stmt)
{

    if(isa<IOStatement>(stmt))
    {
        generateIOStatement(dynamic_cast<IOStatement *>(stmt));
    }
    else if(isa<GraphicsStatement>(stmt))
    {
        generateGraphicsStatement(dynamic_cast<GraphicsStatement *>(stmt));
    }
    else if(isa<AssignmentStmt>(stmt))
    {
        generateAssignmentStmt(dynamic_cast<AssignmentStmt*>(stmt));
    }
    else if(isa<IfStmt>(stmt))
    {
        generateIfStmt(dynamic_cast<IfStmt *>(stmt));
    }
    else if(isa<WhileStmt>(stmt))
    {
        generateWhileStmt(dynamic_cast<WhileStmt *>(stmt));
    }
    else if(isa<ForAllStmt>(stmt))
    {
        generateForAllStmt(dynamic_cast<ForAllStmt *>(stmt));
    }
    else if(isa<LabelStmt>(stmt))
    {
        generateLabelStmt(dynamic_cast<LabelStmt*>(stmt));
    }
    else if(isa<GotoStmt>(stmt))
    {
        generateGotoStmt(dynamic_cast<GotoStmt*>(stmt));
    }
    else if(isa<DelegationStmt>(stmt))
    {
        generateDelegationStmt(dynamic_cast<DelegationStmt *>(stmt));
    }
    else if(isa<ReturnStmt>(stmt))
    {
        generateReturnStmt(dynamic_cast<ReturnStmt *>(stmt));
    }
    else if(isa<BlockStmt>(stmt))
    {
        generateBlockStmt(dynamic_cast<BlockStmt *>(stmt));
    }
    else if(isa<InvokationStmt>(stmt))
    {
        generateInvokationStmt(dynamic_cast<InvokationStmt *>(stmt));
    }
    else if(isa<EventStatement>(stmt))
    {
        generateEventStatement(dynamic_cast<EventStatement *>(stmt));
    }
    else
    {
        throw new CompilerException(stmt, UnimplementedStatementForm);
    }
}
void CodeGenerator::generateIOStatement(IOStatement *stmt)
{
    if(isa<PrintStmt>(stmt))
    {
        generatePrintStmt(dynamic_cast<PrintStmt *>(stmt));
        return;
    }
    if(isa<ReadStmt>(stmt))
    {
        generateReadStmt(dynamic_cast<ReadStmt *>(stmt));
        return;
    }
}
void CodeGenerator::generatePrintStmt(PrintStmt *stmt)
{
    QString fileVar = "";
    if(stmt->fileObject() != NULL)
    {
        fileVar = _asm.uniqueVariable();
        generateExpression(stmt->fileObject());
        gen(stmt, "popl " + fileVar);
    }
    int n = stmt->argCount();
    for(int i=0; i<n; i++)
    {
        if(stmt->_widths[i].isNull())
        {
            generateExpression(stmt->arg(i));
            if(fileVar != "")
            {
                gen(stmt, "pushl "+ fileVar);
                gen(stmt, "callm " + QString::fromStdWString(L"اكتب"));
            }
            else
            {
                gen(stmt, "callex print");
            }
        }
        else
        {
            generateExpression(stmt->_widths[i].data());
            generateExpression(stmt->arg(i));
            if(fileVar != "")
            {
                gen(stmt, "pushl "+ fileVar);
                gen(stmt, "callm "+ QString::fromStdWString(L"اكتب.بعرض"));
            }
            else
            {
                gen(stmt,"callex printw");
            }
        }
    }
    if(!stmt->printOnSameLine)
    {
        if(fileVar != "")
        {
            QString emptyStrConstant = _asm.makeStringConstant("");
            gen(stmt, "pushc "+ emptyStrConstant);
            gen(stmt, "pushl "+ fileVar);
            gen(stmt, "callm "  + QString::fromStdWString(L"اطبع.سطر"));
        }
        else
        {
            gen(stmt,"pushc new_line");
            gen(stmt, "callex print");
        }
    }
}
void CodeGenerator::generateReadStmt(ReadStmt *stmt)
{
    struct GenerateRvalue : public Thunk
    {
        AST *_src; CodeGenerator *_g; QString _v;
        GenerateRvalue(AST *src, CodeGenerator *g, QString v):_src(src),_g(g), _v(v){}
        void operator()()
        {
            _g->gen(_src, "pushl " + _v);
        }
    };

    if(stmt->fileObject() != NULL)
    {
        if(stmt->prompt != NULL)
            throw CompilerException(stmt, ReadFromCannotContainAPrompt);
        if(stmt->variableCount() !=1)
            throw CompilerException(stmt, ReadFromCanReadOnlyOneVariable);
        AssignableExpression *lvalue = stmt->variable(0);

        // We first read the data into a temporary variable,
        // then we generate the equivalent of a hidden assignment statement that assigns
        // the value of the temp. variable to the lvalue that we wanted to read.
        QString readVar = _asm.uniqueVariable();
        defineInCurrentScope(readVar);
        bool readNum = stmt->readNumberFlags[0];

        QString fileVar = _asm.uniqueVariable();
        generateExpression(stmt->fileObject());
        gen(stmt, "popl "+fileVar);

        gen(stmt, "pushl " + fileVar);
        gen(stmt, "callm " + QString::fromStdWString(L"اقرأ.سطر"));
        if(readNum)
        {
            gen(lvalue, "callex to_num");
        }
        gen(lvalue, "popl " + readVar);
        // The read statement is one of the methods to introduce a new variable
        // into scope,but since in this current implementation generateAssignmentToLvalue(..)
        // already adds the lvalue to the current scope if it's an identifier, we don't need to do so here.
        GenerateRvalue genReadVar(lvalue, this, readVar);

        generateAssignmentToLvalue(lvalue, lvalue, genReadVar);

        return;
    }
    if(stmt->prompt !=NULL)
    {
        generateStringConstant(stmt, stmt->prompt);
        gen(stmt, "callex print");
    }

    for(int i=0; i<stmt->variableCount(); i++)
    {
        AssignableExpression *lvalue = stmt->variable(i);
        QString readVar = _asm.uniqueVariable();
        defineInCurrentScope(readVar);

        bool readNum = stmt->readNumberFlags[i];

        if(readNum)
            gen(lvalue, "pushv 1");
        else
            gen(lvalue, "pushv 0");

        gen(lvalue,"callex input");

        gen(lvalue, "popl "+readVar);
        GenerateRvalue genReadVar(lvalue, this, readVar);
        generateAssignmentToLvalue(lvalue, lvalue, genReadVar);
    }
}
void CodeGenerator::generateGraphicsStatement(GraphicsStatement *stmt)
{
    if(isa<DrawPixelStmt>(stmt))
    {
        generateDrawPixelStmt(dynamic_cast<DrawPixelStmt *>(stmt));
        return;
    }
    if(isa<DrawLineStmt>(stmt))
    {
        generateDrawLineStmt(dynamic_cast<DrawLineStmt *>(stmt));
        return;
    }
    if(isa<DrawRectStmt>(stmt))
    {
        generateDrawRectStmt(dynamic_cast<DrawRectStmt *>(stmt));
        return;
    }
    if(isa<DrawCircleStmt>(stmt))
    {
        generateDrawCircleStmt(dynamic_cast<DrawCircleStmt *>(stmt));
        return;
    }
    if(isa<DrawSpriteStmt>(stmt))
    {
        generateDrawSpriteStmt(dynamic_cast<DrawSpriteStmt *>(stmt));
        return;
    }
    if(isa<ZoomStmt>(stmt))
    {
        generateZoomStmt(dynamic_cast<ZoomStmt *>(stmt));
        return;
    }
}
void CodeGenerator::generateDrawPixelStmt(DrawPixelStmt *stmt)
{
    if(stmt->color() == NULL)
        gen(stmt, "pushv -1");
    else
        generateExpression(stmt->color());
    generateExpression(stmt->y());
    generateExpression(stmt->x());
    gen(stmt, "callex drawpixel");
}
void CodeGenerator::generateDrawLineStmt(DrawLineStmt *stmt)
{
    if(stmt->color() == NULL)
        gen(stmt, "pushv -1");
    else
        generateExpression(stmt->color());
    generateExpression(stmt->y2());
    generateExpression(stmt->x2());
    generateExpression(stmt->y1());
    generateExpression(stmt->x1());

    gen(stmt, "callex drawline");
}
void CodeGenerator::generateDrawRectStmt(DrawRectStmt *stmt)
{
    if(stmt->filled())
        generateExpression(stmt->filled());
    else
        gen(stmt, "pushv false");
    if(stmt->color() == NULL)
        gen(stmt, "pushv -1");
    else
        generateExpression(stmt->color());

    generateExpression(stmt->y2());
    generateExpression(stmt->x2());
    generateExpression(stmt->y1());
    generateExpression(stmt->x1());

    gen(stmt, "callex drawrect");
}

void CodeGenerator::generateDrawCircleStmt(DrawCircleStmt *stmt)
{

    if(stmt->filled())
        generateExpression(stmt->filled());
    else
        gen(stmt, "pushv false");

    if(stmt->color() == NULL)
        gen(stmt, "pushv -1");
    else
        generateExpression(stmt->color());

    generateExpression(stmt->radius());
    generateExpression(stmt->cy());
    generateExpression(stmt->cx());

    gen(stmt, "callex drawcircle");
}

void CodeGenerator::generateDrawSpriteStmt(DrawSpriteStmt *stmt)
{
    generateExpression(stmt->y());
    generateExpression(stmt->x());
    generateExpression(stmt->sprite());
    gen(stmt, "callex drawsprite");
}
void CodeGenerator::generateZoomStmt(ZoomStmt *stmt)
{
    generateExpression(stmt->y2());
    generateExpression(stmt->x2());
    generateExpression(stmt->y1());
    generateExpression(stmt->x1());

    gen(stmt, "callex zoom");
}

void CodeGenerator::generateEventStatement(EventStatement *stmt)
{
    QString type;
    if(stmt->type == KalimatKeyDownEvent)
        type = "keydown";
    else if(stmt->type == KalimatKeyUpEvent)
        type = "keyup";
    else if(stmt->type == KalimatKeyPressEvent)
        type = "keypress";
    else if(stmt->type == KalimatMouseDownEvent)
        type = "mousedown";
    else if(stmt->type == KalimatMouseUpEvent)
        type = "mouseup";
    else if(stmt->type == KalimatMouseMoveEvent)
        type = "mousemove";
    else if(stmt->type == KalimatSpriteCollisionEvent)
        type = "collision";
    gen(stmt, QString("regev %1,%2").arg(type).arg(stmt->handler()->name));
}


void CodeGenerator::generateAssignmentStmt(AssignmentStmt *stmt)
{
    AssignableExpression *lval = stmt->variable();

    struct GenerateExpr : public Thunk
    {
        CodeGenerator *_g; Expression *_e;
        GenerateExpr(CodeGenerator *g, Expression *e):_g(g),_e(e) {}
        void operator() (){ _g->generateExpression(_e); }
    } myGen(this, stmt->value());

    generateAssignmentToLvalue(stmt, lval, myGen);
}

void CodeGenerator::generateAssignmentToLvalue(AST *src, AssignableExpression *lval,
                                               Thunk &genValue)
{
    if(isa<Identifier>(lval))
    {
        Identifier *variable = (Identifier *) lval;
        genValue();
        if(declaredGlobalVariables.contains(variable->name))
            gen(src, "popg "+variable->name);
        else
        {
            defineInCurrentScope(variable->name);
            gen(src, "popl "+variable->name);
        }
    }
    else if(isa<Idafa>(lval))
    {
        Idafa *fieldAccess = (Idafa *) lval;
        generateExpression(fieldAccess->modaf_elaih());
        genValue();
        gen(src, "setfld "+ fieldAccess->modaf()->name);
    }
    else if(isa<ArrayIndex>(lval))
    {
        ArrayIndex *arri= (ArrayIndex *) lval;
        generateExpression(arri->array());
        generateExpression(arri->index());
        genValue();
        gen(src, "setarr");
    }
    else if(isa<MultiDimensionalArrayIndex>(lval))
    {
        MultiDimensionalArrayIndex *arri = (MultiDimensionalArrayIndex *) lval;
        generateExpression(arri->array());
        generateArrayFromValues(lval, arri->indexes());
        genValue();
        gen(src, "setmdarr");
    }
    else
    {
        throw CompilerException(src, LValueFormNotImplemented).arg(lval->toString());
    }
}

void CodeGenerator::generateIfStmt(IfStmt *stmt)
{
    generateExpression(stmt->condition());
    QString trueLabel = _asm.uniqueLabel();
    QString falseLabel = _asm.uniqueLabel();
    QString endLabel = _asm.uniqueLabel();

    if(stmt->elsePart()==NULL)
        falseLabel = endLabel;

    gen(stmt, "if "+ trueLabel+","+falseLabel);
    gen(stmt->thenPart(), trueLabel+":");
    generateStatement(stmt->thenPart());
    gen(stmt->thenPart(), "jmp "+endLabel);
    if(stmt->elsePart()!= NULL)
    {
        gen(stmt->elsePart(), falseLabel+":");
        generateStatement(stmt->elsePart());
    }
    gen(stmt, endLabel+":");
}

void CodeGenerator::generateWhileStmt(WhileStmt *stmt)
{
    /*
      while(cond)
         stmt

      label0
      cond
      if label1 label2
      label1:
      stmt
      jmp label0
      label2:
    */
    QString testLabel = _asm.uniqueLabel();
    QString doLabel = _asm.uniqueLabel();
    QString endLabel = _asm.uniqueLabel();

    gen(stmt, testLabel+":");
    generateExpression(stmt->condition());
    gen(stmt->condition(), "if "+doLabel+","+endLabel);
    gen(stmt->statement(), doLabel+":");
    generateStatement(stmt->statement());
    gen(stmt, "jmp "+testLabel);
    gen(stmt, endLabel+":");
}

void CodeGenerator::generateForAllStmt(ForAllStmt *stmt)
{
    /*
      forall(var, from, to)
          stmt

      from
      popl var
      label0:
      pushl var
      to
      gt
      if label2, label1
      label1:
      stmt
      pushl var
      pushv 1
      add
      popl var
      jmp label0
      label2:
    */

    QString testLabel = _asm.uniqueLabel();
    QString doLabel = _asm.uniqueLabel();
    QString endLabel = _asm.uniqueLabel();

    generateExpression(stmt->from());
    gen(stmt->variable(), "popl " + stmt->variable()->name);
    gen(stmt, testLabel+":");
    gen(stmt->variable(), "pushl "+ stmt->variable()->name);
    generateExpression(stmt->to());
    gen(stmt, "gt");
    gen(stmt, "if "+endLabel+","+doLabel);
    gen(stmt, doLabel+":");

    defineInCurrentScope(stmt->variable()->name);

    generateStatement(stmt->statement());
    gen(stmt, "pushl "+ stmt->variable()->name);
    gen(stmt, "pushv 1");
    gen(stmt, "add");
    gen(stmt, "popl "+ stmt->variable()->name);
    gen(stmt, "jmp "+testLabel);
    gen(stmt, endLabel+":");
}

void CodeGenerator::generateLabelStmt(LabelStmt *stmt)
{
    Expression *target = stmt->target();
    QString labelName = target->toString();
    if(scopeStack.top().labels.contains(labelName))
    {
        throw CompilerException(stmt, DuplicateLabel).arg(labelName).arg(getCurrentFunctionNameFormatted());
    }
    scopeStack.top().labels.insert(labelName);

    if(isa<NumLiteral>(target))
    {
        gen(stmt, labelName + ": ");
    }
    else if(isa<Identifier>(target))
    {
        gen(stmt, labelName + ": ");
    }
    else
    {
        throw CompilerException(stmt, TargetOfLabelMustBeNumberOrIdentifier);
    }
}

void CodeGenerator::generateGotoStmt(GotoStmt *stmt)
{
    if(stmt->numericTarget())
    {
        gen(stmt, "jmp "+ stmt->numericTarget()->toString());
    }
    else
    {
        gen(stmt, "jmp "+ stmt->idTarget()->name);
    }
}

void CodeGenerator::generateReturnStmt(ReturnStmt *stmt)
{
    bool notFunc = false;
    if(scopeStack.empty())
        notFunc = true;
    else if(isa<MethodDecl>(scopeStack.top().proc))
    {
        notFunc = !((MethodDecl *) scopeStack.top().proc)->isFunctionNotProcedure;
    }
    else if(!isa<FunctionDecl>(scopeStack.top().proc))
        notFunc = true;
    if(notFunc)
        throw CompilerException(stmt, ReturnCanBeUsedOnlyInFunctions);
    generateExpression(stmt->returnVal());
    gen(stmt,"ret");
}

void CodeGenerator::generateDelegationStmt(DelegationStmt *stmt)
{
    IInvokation *expr = stmt->invokation();
    if(isa<Invokation>(expr))
    {
        generateInvokation(dynamic_cast<Invokation *>(expr), TailCall);
        return;
    }
    if(isa<MethodInvokation>(expr))
    {
        generateMethodInvokation(dynamic_cast<MethodInvokation *>(expr), TailCall);
        return;
    }

    throw CompilerException(expr, UnimplementedInvokationForm).arg(expr->toString());
}

void CodeGenerator::generateBlockStmt(BlockStmt *stmt)
{
    for(int i=0; i<stmt->statementCount(); i++)
    {
        generateStatement(stmt->statement(i));
    }
}

void CodeGenerator::generateInvokationStmt(InvokationStmt *stmt)
{
    generateExpression(stmt->expression());
}

void CodeGenerator::generateExpression(Expression *expr)
{
    if(isa<BinaryOperation>(expr))
    {
        generateBinaryOperation(dynamic_cast<BinaryOperation *>(expr));
        return;
    }
    if(isa<IsaOperation>(expr))
    {
        generateIsaOperation(dynamic_cast<IsaOperation *>(expr));
        return;
    }
    if(isa<UnaryOperation>(expr))
    {
        generateUnaryOperation(dynamic_cast<UnaryOperation *>(expr));
        return;
    }
    if(isa<Identifier>(expr))
    {
        generateIdentifier(dynamic_cast<Identifier *>(expr));
        return;
    }
    if(isa<NumLiteral>(expr))
    {
        generateNumLiteral(dynamic_cast<NumLiteral *>(expr));
        return;
    }
    if(isa<StrLiteral>(expr))
    {
        generateStrLiteral(dynamic_cast<StrLiteral *>(expr));
        return;
    }
    if(isa<NullLiteral>(expr))
    {
        generateNullLiteral(dynamic_cast<NullLiteral *>(expr));
        return;
    }
    if(isa<BoolLiteral>(expr))
    {
        generateBoolLiteral(dynamic_cast<BoolLiteral *>(expr));
        return;
    }
    if(isa<ArrayLiteral>(expr))
    {
        generateArrayLiteral(dynamic_cast<ArrayLiteral *>(expr));
        return;
    }
    if(isa<Invokation>(expr))
    {
        generateInvokation(dynamic_cast<Invokation *>(expr));
        return;
    }
    if(isa<MethodInvokation>(expr))
    {
        generateMethodInvokation(dynamic_cast<MethodInvokation *>(expr));
        return;
    }
    if(isa<Idafa>(expr))
    {
        generateIdafa(dynamic_cast<Idafa *>(expr));
        return;
    }
    if(isa<ArrayIndex>(expr))
    {
        generateArrayIndex(dynamic_cast<ArrayIndex *>(expr));
        return;
    }
    if(isa<MultiDimensionalArrayIndex>(expr))
    {
        generateMultiDimensionalArrayIndex(dynamic_cast<MultiDimensionalArrayIndex *>(expr));
        return;
    }
    if(isa<ObjectCreation>(expr))
    {
        generateObjectCreation(dynamic_cast<ObjectCreation *>(expr));
        return;
    }
    throw CompilerException(expr, UnimplementedExpressionForm).arg(expr->toString());
}

void CodeGenerator::generateBinaryOperation(BinaryOperation *expr)
{
    if(expr->_operator == "and")
    {
        // if(op1)
        //     op2
        // else
        //     false
        // =>
        // <op1>
        // if goon, else
        // goon:
        // <op2>
        // jmp theend
        // else:
        // pushv false
        // theend:

        QString goOn = _asm.uniqueLabel();
        QString elSe = _asm.uniqueLabel();
        QString theEnd = _asm.uniqueLabel();

        generateExpression(expr->operand1());
        gen(expr, "if " + goOn + "," + elSe);
        gen(expr, goOn+":");
        generateExpression(expr->operand2());
        gen(expr, "jmp " + theEnd);
        gen(expr, elSe+":");
        gen(expr->operand1(), "pushv false");
        gen(expr, theEnd + ":");
    }
    else if(expr->_operator == "or")
    {
        // if(op1)
        //     op2
        // else
        //     false
        // =>
        // <op1>
        // if else,goon
        // goon:
        // <op2>
        // jmp theend
        // else:
        // pushv true
        // theend:

        QString goOn = _asm.uniqueLabel();
        QString elSe = _asm.uniqueLabel();
        QString theEnd = _asm.uniqueLabel();

        generateExpression(expr->operand1());
        gen(expr, "if " + elSe + "," + goOn);
        gen(expr, goOn+":");
        generateExpression(expr->operand2());
        gen(expr, "jmp " + theEnd);
        gen(expr, elSe+":");
        gen(expr->operand1(), "pushv true");
        gen(expr, theEnd + ":");
    }
    else
    {
        generateExpression(expr->operand1());
        generateExpression(expr->operand2());
        gen(expr, expr->_operator);
    }
}

void CodeGenerator::generateIsaOperation(IsaOperation *expr)
{
    generateExpression(expr->expression());
    QString typeId = expr->type()->name;
    gen(expr, "isa " + typeId);
}

void CodeGenerator::generateUnaryOperation(UnaryOperation *expr)
{
    generateExpression(expr->operand());
    gen(expr, expr->_operator);
}

void CodeGenerator::generateIdentifier(Identifier *expr)
{
    if(!scopeStack.empty() && scopeStack.top().bindings.contains(expr->name))
        gen(expr, "pushl "+expr->name);
    else if(declaredGlobalVariables.contains(expr->name))
        gen(expr, "pushg "+expr->name);
    else
        throw CompilerException(expr, UndefinedVariable).arg(expr->name);

}

void CodeGenerator::generateNumLiteral(NumLiteral *expr)
{
    if(!expr->valueRecognized)
    {
        throw CompilerException(expr, UnacceptableNumberLiteral).arg(expr->toString());
    }
    if(expr->longNotDouble)
        gen(expr, "pushv "+ QString("%1").arg(expr->lValue));
    else
        gen(expr, "pushv "+ QString("%1").arg(expr->dValue));
}

void CodeGenerator::generateStrLiteral(StrLiteral *expr)
{
    generateStringConstant(expr, expr->value);
}

void CodeGenerator::generateNullLiteral(NullLiteral *expr)
{
    gen(expr, "pushnull");
}

void CodeGenerator::generateBoolLiteral(BoolLiteral *expr)
{
    if(expr->value)
        gen(expr, "pushv true");
    else
        gen(expr, "pushv false");
}

void CodeGenerator::generateArrayLiteral(ArrayLiteral *expr)
{
    QString newVar = generateArrayFromValues(expr, expr->dataVector());
}

void CodeGenerator::generateInvokation(Invokation *expr, MethodCallStyle style)
{
    for(int i=expr->argumentCount()-1; i>=0; i--)
    {
        generateExpression(expr->argument(i));
    }
    if(style == TailCall)
    {
        gen(expr->functor(), "tail");
    }
    gen(expr->functor(), QString("call %1,%2").arg(expr->functor()->toString()).arg(expr->argumentCount()));

}

void CodeGenerator::generateMethodInvokation(MethodInvokation *expr, MethodCallStyle style)
{
    for(int i=expr->argumentCount()-1; i>=0; i--)
    {
        generateExpression(expr->argument(i));
    }
    generateExpression(expr->receiver());
    if(style == TailCall)
    {
        gen(expr->methodSelector(), "tail");
    }
    gen(expr->methodSelector(), QString("callm %1,%2").arg(expr->methodSelector()->name).arg(expr->argumentCount()+1));
}

void CodeGenerator::generateIdafa(Idafa *expr)
{
    generateExpression(expr->modaf_elaih());
    gen(expr->modaf(), "getfld "+expr->modaf()->name);
}

void CodeGenerator::generateArrayIndex(ArrayIndex *expr)
{
    generateExpression(expr->array());
    generateExpression(expr->index());
    gen(expr, "getarr");
}

void CodeGenerator::generateMultiDimensionalArrayIndex(MultiDimensionalArrayIndex *expr)
{
    generateExpression(expr->array());
    generateArrayFromValues(expr, expr->indexes());
    gen(expr, "getmdarr");
}

QString CodeGenerator::generateArrayFromValues(AST *src, QVector<QSharedPointer<Expression> >values)
{
    QString newVar = _asm.uniqueVariable();
    gen(src, "pushv ", values.count());
    gen(src, "newarr");
    gen(src, "popl "+ newVar);
    // Notice that Kalimat arrays are one-based, not zero-based
    for(int i=1; i<=values.count(); i++)
    {
       Expression *value = values[i-1].data();
       gen(value, "pushl "+ newVar);
       gen(value, "pushv ", i);
       generateExpression(value);
       gen(value, "setarr");
    }
    gen(src,"pushl " + newVar);
    return newVar;
}

void CodeGenerator::generateObjectCreation(ObjectCreation *expr)
{
    gen(expr, "new "+expr->className()->name);
}

void CodeGenerator::generateStringConstant(AST *src, QString str)
{
    QString constId = _asm.makeStringConstant(str);

    gen(src, "pushc "+ constId);
}

void CodeGenerator::gen(QString str)
{
    _asm.gen(str);
}

void CodeGenerator::gen(QString str, int i)
{
    _asm.gen(str, i);
}

void CodeGenerator::gen(QString str, double d)
{
    _asm.gen(str, d);
}

void CodeGenerator::gen(AST *src,QString str)
{
    CodePosition pos;
    pos.doc = currentCodeDoc;
    pos.pos = src->getPos().Pos;
    pos.ast = src;
    PositionInfo[codePosKeyCount] = pos;
    _asm.genWithMetaData(codePosKeyCount, str);
    codePosKeyCount++;
}

void CodeGenerator::gen(AST *src,QString str, int i)
{
    gen(src, QString(str)+" "+ QString("%1").arg(i));
}

void CodeGenerator::gen(AST *src,QString str, double d)
{
    gen(src, QString(str)+" "+ QString("%1").arg(d));
}

QString CodeGenerator::getCurrentFunctionNameFormatted()
{
    QString fname = scopeStack.top().proc->procName()->name;
    if(fname == "%main")
        fname = _ws(L"البرنامج الرئيسي");
    else
        fname = QString("'%1'").arg(fname);
    return fname;
}

QMap<CompilerError, QString> CompilerException::errorMap;

CompilerException::CompilerException(AST *source, CompilerError error)
{
    this->source = source;
    this->message = translateErrorMessage(error);
}

QString CompilerException::getMessage()
{
    QString ret = message;
    for(int i=0; i<args.count(); i++)
        ret = ret.arg(args.at(i));
    return ret;
}

QString CompilerException::translateErrorMessage(CompilerError error)
{
    if(errorMap.empty())
    {
        LineIterator in = Utils::readResourceTextFile(":/compiler_error_map.txt");
        int i=0;
        while(!in.atEnd())
        {
            QString val = in.readLine().trimmed();
            errorMap[(CompilerError) i++] = val;
        }
        in.close();
    }
    return errorMap[error];
}

CompilerException &CompilerException::arg(QString arg)
{
    args.append(arg);
    return *this;
}