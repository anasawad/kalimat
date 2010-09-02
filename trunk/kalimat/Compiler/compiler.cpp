/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "Lexer/lexer_incl.h"
#include "Lexer/kalimatlexer.h"
#include "Parser/parser_incl.h"
#include "Parser/kalimatast.h"
#include "Parser/kalimatparser.h"
#include "codedocument.h"
#include "codegenerator.h"



#include "compiler.h"

Compiler::Compiler()
{
    
}
AST *parseModule(Parser *p)
{
    return ((KalimatParser *) p)->module();
}
Program *Compiler::loadProgram(QString path)
{
    parser.init(loadFileContents(path), &lexer);
    Program *p = (Program *) parser.parse();

    for(int i=0; i<p->usedModuleCount(); i++)
    {
        QString m2 = p->usedModule(i)->value;
        QString fullPath = getPathOfModule(m2, path);
        if(!loadedModules.contains(fullPath))
            loadModule(fullPath);
        Module *importedMod = loadedModules[fullPath];
        /*
        for(int i=0; i<importedMod->declCount(); i++)
        {
            Declaration *decl = importedMod->decl(i);
            if(decl->isPublic())
            {
                p->addElement(decl);
            }
        }
        //*/
    }
    return p;
}

Module *Compiler::loadModule(QString path)
{
    parser.init(loadFileContents(path), &lexer);
    Module *m = (Module *) parser.parse(parseModule);
    loadedModules[path] = m;
    for(int i=0; i<m->usedModuleCount(); i++)
    {
        QString m2 = m->usedModule(i)->value;
        QString fullPath = getPathOfModule(m2, path);
        if(!loadedModules.contains(fullPath))
            loadModule(fullPath);
        Module *importedMod = loadedModules[fullPath];
        /*
        for(int i=0; i<importedMod->declCount(); i++)
        {
            Declaration *decl = importedMod->decl(i);
            if(decl->isPublic())
            {
                m->addDecl(decl);
            }
        }
        //*/
    }
    return m;
}
void Compiler::generateAllLoadedModules()
{
    for(QMap<QString,Module *>::iterator i= loadedModules.begin(); i!=loadedModules.end(); ++i)
    {
        generator.compileModule(*i, NULL);
    }
}

QString Compiler::getPathOfModule(QString name, QString pathOfImportingModule)
{
    QString directory = directoryFromFullPath(pathOfImportingModule);
    QString ret = combinePath(directory, name);
    return ret;
}

QString Compiler::directoryFromFullPath(QString path)
{
    return QFileInfo(path).absolutePath();
}
QString Compiler::combinePath(QString parent, QString child)
{
    return QFileInfo(parent, child).absoluteFilePath();
}

QString Compiler::CompileFromCode(QString source, CodeDocument *doc)
{
    parser.init(source, &lexer);
    Program *p = (Program *) parser.parse();

    if(p->usedModuleCount()!=0)
        throw CompilerException(p, ProgramsCannotUseExternalModulesWithoutSavingThemFirst);

    generator.generate(p, doc);
    return generator.getOutput();
}

QString Compiler::CompileFromFile(QString pathToMainCompilationUnit, CodeDocument *doc)
{
    Program *p = loadProgram(pathToMainCompilationUnit);
    generateAllLoadedModules();
    generator.generate(p, doc);
    return generator.getOutput();
}
QString Compiler::loadFileContents(QString path)
{
    QFile file(path);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream out(&file);
    QString ret = out.readAll();
    file.close();
    return ret;
}