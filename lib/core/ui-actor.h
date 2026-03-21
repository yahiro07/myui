#pragma once
#include "core-types.h"
#include "node-handle.h"
#include <source_location>

namespace myui {

void drawNodeWithTranslate(DrawingContext &dc, internal::Node *node,
                           internal::NodeBox &box, InputState &gInputState);

class UiActor {
private:
  using PassOps = internal::PassOps;

private:
  PassOps &passOps;

public:
  UiActor(PassOps &phaseOps) : passOps(phaseOps) {}
  UiActor(const UiActor &) = delete;
  UiActor &operator=(const UiActor &) = delete;

public:
  NodeHandle
  rootBox(int w, int h, UiLayoutMode layout = LA_Default,
          std::source_location loc = std::source_location::current()) {
    auto node = passOps.createNodeWithId(w, h, layout, loc, true);
    return NodeHandle(*node, passOps);
  }

  NodeHandle box(int w, int h, UiLayoutMode layout = LA_Default,
                 std::source_location loc = std::source_location::current()) {
    auto node = passOps.createNodeWithId(w, h, layout, loc, false);
    return NodeHandle(*node, passOps);
  }
};

} // namespace myui