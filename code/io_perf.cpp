#include <ostream>

struct NullBuffer : std::streambuf {
  int overflow(int c) override { return c; }
};

static NullBuffer null_buffer;
static std::ostream null_stream(&null_buffer);