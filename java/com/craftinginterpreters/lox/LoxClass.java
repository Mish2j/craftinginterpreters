//> Classes lox-class
package com.craftinginterpreters.lox;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/* Classes lox-class < Classes lox-class-callable
class LoxClass {
*/
//> lox-class-callable
class LoxClass extends LoxInstance implements LoxCallable {
//< lox-class-callable
  final String name;
//> Inheritance lox-class-superclass-field
  final LoxClass superclass;
//< Inheritance lox-class-superclass-field
/* Classes lox-class < Classes lox-class-methods

  LoxClass(String name) {
    this.name = name;
  }
*/
//> lox-class-methods
  private final Map<String, LoxFunction> methods;
 
  final LoxClass metaclass;

/* Classes lox-class-methods < Inheritance lox-class-constructor
  LoxClass(String name, Map<String, LoxFunction> methods) {
*/
//> Inheritance lox-class-constructor
  LoxClass(String name, LoxClass superclass,
           Map<String, LoxFunction> methods,
          LoxClass metaclass
          ) {
    super(metaclass);
    this.superclass = superclass;
//< Inheritance lox-class-constructor
    this.name = name;
    this.methods = methods;
    this.metaclass = metaclass;
  }
//< lox-class-methods
  LoxFunction findMethodTopDown(String name) {
    List<LoxClass> chain = chainFromRoot();
    for (LoxClass klass : chain) {
      LoxFunction method = klass.methods.get(name);
      if (method != null) return method;
    }
    return null;
  }

  // For inner(): find next matching method below definingClass, down toward runtimeClass(this).
  LoxFunction findInnerTarget(String methodName, LoxClass definingClass, LoxClass runtimeClass) {
    List<LoxClass> chain = runtimeClass.chainFromRoot();

    int start = -1;
    for (int i = 0; i < chain.size(); i++) {
      if (chain.get(i) == definingClass) {
        start = i;
        break;
      }
    }
    if (start == -1) return null; // should not happen if definingClass is on chain

    for (int i = start + 1; i < chain.size(); i++) {
      LoxClass klass = chain.get(i);
      LoxFunction method = klass.methods.get(methodName);
      if (method != null) return method;
    }
    return null;
  }
//> lox-class-find-method
  LoxFunction findMethod(String name) {
    List<LoxClass> chain = inheritanceChain();
    
    if (methods.containsKey(name)) {
      return methods.get(name);
    }

//> Inheritance find-method-recurse-superclass
    if (superclass != null) {
      return superclass.findMethod(name);
    }

//< Inheritance find-method-recurse-superclass
    return null;
  }
//< lox-class-find-method

  @Override
  public String toString() {
    return name;
  }
//> lox-class-call-arity
  @Override
  public Object call(Interpreter interpreter,
                     List<Object> arguments) {
    LoxInstance instance = new LoxInstance(this);
//> lox-class-call-initializer
    LoxFunction initializer = findMethod("init");
    if (initializer != null) {
      initializer.bind(instance).call(interpreter, arguments);
    }

//< lox-class-call-initializer
    return instance;
  }

  @Override
  public int arity() {
/* Classes lox-class-call-arity < Classes lox-initializer-arity
    return 0;
*/
//> lox-initializer-arity
    LoxFunction initializer = findMethod("init");
    if (initializer == null) return 0;
    return initializer.arity();
//< lox-initializer-arity
  }
//< lox-class-call-arity

  List<LoxClass> inheritanceChain() {
    List<LoxClass> chain = new ArrayList<>();
    LoxClass current = this;
    while (current != null) {
      chain.add(0, current); // prepend
      current = current.superclass;
    }
    return chain;
  }
   List<LoxClass> chainFromRoot() {
    List<LoxClass> chain = new ArrayList<>();
    LoxClass current = this;
    while (current != null) {
      chain.add(0, current);
      current = current.superclass;
    }
    return chain;
  }
}
