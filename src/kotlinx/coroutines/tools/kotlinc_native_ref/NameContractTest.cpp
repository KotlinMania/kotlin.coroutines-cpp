#include "org/jetbrains/kotlin/name/Name.hpp"
#include "kotlin/native/Runtime.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>

using org::jetbrains::kotlin::name::Name;

// Contract cases deliberately distinguish UTF-16 from UTF-8/code-point order,
// and the source's permissive factory from identifier validation.
int main() {
  const auto ordinary = Name::identifier(u"result");
  const auto special = Name::special(u"<result>");
  assert(kotlin::native::identity_hash_code(nullptr) == 0);
  const auto ordinary_identity = kotlin::native::identity_hash_code(&ordinary);
  assert(kotlin::native::identity_hash_code(&ordinary) == ordinary_identity);
  const std::string cpp_text = "ordinary C++ storage";
  const auto cpp_identity = kotlin::native::identity_hash_code(&cpp_text);
  assert(kotlin::native::identity_hash_code(&cpp_text) == cpp_identity);
  assert(ordinary.get_identifier() == u"result");
  assert(ordinary.to_string() == ordinary.as_string());
  assert(ordinary.as_string_strip_special_markers() == u"result");
  assert(ordinary.get_identifier_or_null_if_special() == u"result");
  assert(special.as_string_strip_special_markers() == u"result");
  assert(!special.get_identifier_or_null_if_special());
  assert(Name::guess_by_first_character(u"").as_string().empty());
  assert(Name::guess_by_first_character(u"<x>").is_special());
  assert(!Name::guess_by_first_character(u"x").is_special());
  assert(Name::is_valid_identifier(u"9 name$"));
  assert(!Name::is_valid_identifier(u""));
  assert(!Name::is_valid_identifier(u"<x>"));
  for (const auto* text : {u"a.b", u"a;b", u"a[b", u"a/b"}) {
    assert(!Name::is_valid_identifier(text));
    assert(!Name::identifier_if_valid(text));
  }
  assert(Name::identifier_if_valid(u"9 name$")->get_identifier() == u"9 name$");
  assert(Name::identifier(u"a.b").get_identifier() == u"a.b");
  assert(Name::special(u"<xy").as_string_strip_special_markers() == u"x");
  assert(Name::special(u"<>").as_string_strip_special_markers().empty());
  try {
    special.get_identifier();
    assert(false);
  } catch (const std::logic_error& e) {
    assert(std::string(e.what()) == "not identifier: <result>");
  }
  try {
    Name::special(u"result");
    assert(false);
  } catch (const std::invalid_argument& e) {
    assert(std::string(e.what()) == "special name must start with '<': result");
  }
  try {
    Name::special(u"<").as_string_strip_special_markers();
    assert(false);
  } catch (const std::out_of_range&) {
  }
  assert(ordinary.equals(std::any(Name::identifier(u"result"))));
  assert(!ordinary.equals(std::any{}));
  assert(!ordinary.equals(std::any(std::u16string(u"result"))));
  assert(!Name::identifier(u"<result>").equals(std::any(special)));
  assert(Name::identifier(u"a").compare_to(Name::identifier(u"z")) == -25);
  assert(Name::identifier(u"abc").compare_to(Name::identifier(u"a")) == 2);
  assert(Name::identifier(u"\U00010000").compare_to(Name::identifier(u"\ue000")) == -2048);
  assert(Name::identifier(u"abc").hash_code() == 2986974);
  assert(Name::identifier(u"\U00010000").hash_code() == 54885376);
  assert(Name::identifier(u"zzzzzzzzzz").hash_code() == -1765712960);
  assert(Name::special(u"<x>").hash_code() == Name::identifier(u"<x>").hash_code() + 1);

  // Same ordered observations are emitted by the pinned Java source oracle.
  std::cout << "utf16_order=" << Name::identifier(u"\U00010000").compare_to(Name::identifier(u"\ue000")) << '\n';
  std::cout << "supplementary_hash=" << Name::identifier(u"\U00010000").hash_code() << '\n';
  std::cout << "overflow_hash=" << Name::identifier(u"zzzzzzzzzz").hash_code() << '\n';
  std::cout << "non_identifier_factory=" << Name::identifier(u"a.b").as_string().size() << '\n';
  std::cout << "unclosed_special_strip=" << Name::special(u"<xy").as_string_strip_special_markers().size() << '\n';
  std::cout << "ordinary_vs_special=" << Name::identifier(u"<x>").equals(std::any(Name::special(u"<x>"))) << '\n';
}
