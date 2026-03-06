//> Statements and State environment-class
package com.craftinginterpreters.lox;

import java.util.HashMap;
import java.util.Map;

class Environment {
//> enclosing-field
  final Environment enclosing;
//< enclosing-field
  private final Map<String, Object> values = new HashMap<>();
  private final Object[] slots;
//> environment-constructors
  Environment() {
    enclosing = null;
    slots = null;
  }

  Environment(Environment enclosing, int slotCount) {
    this.enclosing = enclosing;
    this.slots = new Object[slotCount];
  }
//< environment-constructors
//> environment-get

  Object get(Token name) {
    if (values.containsKey(name.lexeme)) {
      return values.get(name.lexeme);
    }
//> environment-get-enclosing

    if (enclosing != null) return enclosing.get(name);
//< environment-get-enclosing

    throw new RuntimeError(name,
        "Undefined variable '" + name.lexeme + "'.");
  }

//< environment-get
//> environment-assign
  void assign(Token name, Object value) {
    if (values.containsKey(name.lexeme)) {
      values.put(name.lexeme, value);
      return;
    }

//> environment-assign-enclosing
    if (enclosing != null) {
      enclosing.assign(name, value);
      return;
    }

//< environment-assign-enclosing
    throw new RuntimeError(name,
        "Undefined variable '" + name.lexeme + "'.");
  }
//< environment-assign
//> environment-define
  void define(String name, Object value) {
    values.put(name, value);
  }
//< environment-define
//> Resolving and Binding ancestor
  Environment ancestor(int distance) {
    Environment environment = this;
    for (int i = 0; i < distance; i++) {
      environment = environment.enclosing; // [coupled]
    }

    return environment;
  }
//< Resolving and Binding ancestor
//> Resolving and Binding get-at
  Object getAt(int distance, int slot) {
    Environment environment = ancestor(distance);
    return environment.slots[slot];
  }
//< Resolving and Binding get-at
//> Resolving and Binding assign-at
  void assignAt(int distance, int slot, Object value) {
    Environment environment = ancestor(distance);
    environment.slots[slot] = value;
  }
//< Resolving and Binding assign-at

  void setSlot(int slot, Object value) {
    slots[slot] = value;
  }

//> omit
  @Override
  public String toString() {
    String result = values.toString();
    if (enclosing != null) {
      result += " -> " + enclosing.toString();
    }

    return result;
  }
//< omit
}
