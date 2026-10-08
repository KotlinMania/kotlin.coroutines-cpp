// Execute actual translated Native IntArray and its consumed HashMap helpers.
// Source ranges are recorded in the production declarations and bodies.
#include "kotlin/IntArray.hpp"
#include "kotlin/collections/ArraysNative.hpp"
#include "kotlin/collections/ArrayUtil.hpp"
#include "kotlin/collections/AbstractListFunctions.hpp"
#include "../../Exceptions.hpp"
#include <cassert>
#include <functional>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <type_traits>

using kotlin::IntArray;
using namespace kotlin::collections;
namespace {
std::string contents(const IntArray& a) {
  std::ostringstream s;
  for (std::int32_t i = 0; i < a.get_size(); ++i) { if (i) s << ','; s << a.get(i); }
  return s.str();
}
IntArray numbers() { return IntArray(4, [](std::int32_t i) { return i + 10; }); }
std::string observe(bool messages, const std::function<std::string()>& operation) {
  try { return operation(); }
  catch (const kotlinx::coroutines::NoSuchElementException& e) { return std::string("NoSuchElementException:") + e.what(); }
  catch (const std::out_of_range& e) { return std::string("IndexOutOfBoundsException") + (messages ? std::string(":") + e.what() : ""); }
  catch (const std::invalid_argument& e) { return std::string("IllegalArgumentException") + (messages ? std::string(":") + e.what() : ""); }
}
void emit(const std::string& key, const std::string& result) { std::cout << key << '=' << result << '\n'; }
std::string tuple(std::int32_t a, std::int32_t b) { return std::to_string(a) + ':' + std::to_string(b); }
static_assert(std::is_abstract_v<IntIterator>);
static_assert(std::is_same_v<decltype(std::declval<IntIterator&>().next_int()), std::int32_t>);
static_assert(std::is_convertible_v<IntIterator*, Iterator<std::any>*>);
}
int main() {
  for (std::int32_t size = 0; size <= 5; ++size) emit("zero:" + std::to_string(size), contents(IntArray(size)));
  for (bool self : {false, true}) for (std::int32_t from = -1; from <= 5; ++from)
    for (std::int32_t to = -1; to <= 5; ++to) for (std::int32_t count = -1; count <= 5; ++count) {
      auto source = numbers(); auto destination = self ? source : IntArray(4, [](std::int32_t) { return 90; });
      const auto before_source = contents(source), before_destination = contents(destination);
      const auto result = observe(false, [&] { array_copy(source, from, destination, to, count); return contents(source) + '|' + contents(destination); });
      if (result == "IndexOutOfBoundsException") assert(contents(source) == before_source && contents(destination) == before_destination);
      emit("copy:" + std::string(self ? "true:" : "false:") + tuple(from, to) + ':' + std::to_string(count), result);
    }
  for (std::int32_t from = -1; from <= 5; ++from) for (std::int32_t to = -1; to <= 5; ++to) {
    auto a = numbers(); emit("fill:" + tuple(from,to), observe(true, [&] { fill(a,71,from,to); return contents(a); }));
    emit("range:" + tuple(from,to), observe(true, [&] { return contents(copy_of_range(numbers(),from,to)); }));
    emit("slice:" + tuple(from,to), observe(false, [&] { return contents(copy_of_uninitialized_elements(numbers(),from,to)); }));
    emit("range-check:" + tuple(from,to), observe(true, [&] { abstract_list::check_range_indexes(from,to,4); return "ok"; }));
    emit("bounds-check:" + tuple(from,to), observe(true, [&] { abstract_list::check_bounds_indexes(from,to,4); return "ok"; }));
  }
  for (std::int32_t i = -1; i <= 5; ++i) {
    emit("get:"+std::to_string(i), observe(false,[&] { return std::to_string(numbers().get(i)); }));
    auto a=numbers(); emit("set:"+std::to_string(i), observe(false,[&] { a.set(i,73); return contents(a); }));
    emit("element-check:"+std::to_string(i), observe(true,[&] { abstract_list::check_element_index(i,4); return "ok"; }));
    emit("position-check:"+std::to_string(i), observe(true,[&] { abstract_list::check_position_index(i,4); return "ok"; }));
  }
  for (std::int32_t size = -1; size <= 7; ++size) {
    emit("resize:"+std::to_string(size), observe(false,[&] { return contents(copy_of(numbers(),size)); }));
    emit("resize-uninitialized:"+std::to_string(size), observe(false,[&] { return contents(copy_of_uninitialized_elements(numbers(),size)); }));
  }
  std::ostringstream order;
  IntArray a(3,[&](std::int32_t i) { if(i) order<<','; order<<i; return i+10; });
  assert(order.str()=="0,1,2"); emit("init-order",order.str());
  auto it=a.iterator(); Iterator<std::any>& widened=*it;
  assert(dynamic_cast<void*>(&widened)==dynamic_cast<void*>(it.get()));
  emit("iterator-same-object","true"); emit("iterator-first",std::to_string(it->next_int()));
  a.set(1,77); emit("iterator-widened-second",std::to_string(std::any_cast<std::int32_t>(widened.next())));
  emit("iterator-third",std::to_string(it->next())); assert(!it->has_next());
  emit("iterator-exhaustion",observe(false,[&]{return std::to_string(it->next_int());}));
  emit("iterator-exhaustion-again",observe(false,[&]{return std::to_string(std::any_cast<std::int32_t>(widened.next()));}));
  auto retained=[] { return IntArray(1,[](std::int32_t){return 31;}).iterator(); }();
  emit("iterator-retained",std::to_string(retained->next_int()));
  auto empty=IntArray(0).iterator(); assert(!empty->has_next());
  emit("iterator-empty",observe(false,[&]{return std::to_string(empty->next_int());}));
  auto original=numbers(); auto copied=copy_of(original); original.set(0,99);
  assert(copied.get(0)==10); emit("copy-independent",contents(copied));
  auto dest=numbers(); auto& returned=copy_into(numbers(),dest); assert(&returned==&dest); emit("default-copy",contents(dest)+":true");
  fill(dest,72); emit("default-fill",contents(dest)); fill(dest,73,2); emit("default-fill-from",contents(dest));
  const auto max=std::numeric_limits<std::int32_t>::max();
  emit("overflow-copy",observe(false,[&]{copy_into(original,dest,0,-1,max);return contents(dest);}));
  emit("overflow-slice",observe(true,[&]{return contents(copy_of_uninitialized_elements(original,-1,max));}));
  for (std::int32_t old=0; old<=64; ++old) for (std::int32_t minimum=0; minimum<=64; ++minimum)
    emit("capacity:"+tuple(old,minimum),std::to_string(abstract_list::new_capacity(old,minimum)));
  const std::int32_t extremes[]={0,1,8,1024,1431655760,1431655765,1431655766, max-9,max-8,max-7,max-1,max};
  for(auto old:extremes) for(auto minimum:extremes)
    emit("capacity:"+tuple(old,minimum),std::to_string(abstract_list::new_capacity(old,minimum)));
}
