#ifndef MATCHER_H
#define MATCHER_H


class matcher{
public:
  matcher(const char* name, const char* mask) : name(name), mask(mask){
  };

  bool match(void){
    if (!try_partial_match())
      return false;
    
    while (*mask == '*'){
      ++mask;
      while (!try_partial_match() && *name != '\0')
        ++name;
    }
    return is_full_match();
  }

private:
  const char* name;
  const char* mask;

  bool is_full_match(void) const{
    return *name == '\0' && *mask == '\0';
  }

  bool patrial_match(void) {
    while (*name != '\0' && (*name == *mask || *mask == '?')){
      ++name;
      ++mask;
    }
    
    return is_full_match() || *mask == '*';
  }

  bool try_partial_match(void){
    auto tmp = *this;
    
    if (tmp.patrial_match()){
      *this = tmp;
      return true;
    }

    return false;
  }
};

#endif