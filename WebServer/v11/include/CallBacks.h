#pragma once

#include <memory>
#include <functional>

class Buffer;
class Connection;

using ConnectionPtr = std::shared_ptr<Connection>;  // destroyed when no ref
using TimerCallBack = std::function<void()>;

using CloseCallBack = std::function<void(const ConnectionPtr&)>;
using WriteCompletedCallBack = std::function<void(const ConnectionPtr&)>;
using ConnectionCallBack = std::function<void(const ConnectionPtr&)>;

using MessageCallBack = std::function<void(const ConnectionPtr&, Buffer*)>;
