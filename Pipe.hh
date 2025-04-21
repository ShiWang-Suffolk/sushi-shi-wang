#pragma once

class Pipe {
private:
  Program *head, *tail;
public:
  Pipe(Program *p) : head(p), tail(p) {};
  Program *hd() { return head; }
  Program *tl() { return tail; }
  void tl(Program *p) { tail = p; }
};

