#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <climits>

//Output: five lines (FIFO, LRU, LFU, OPT)
//Page number(+ or -)
//+ - if page exists in memory
//- - if doesn't exist in memory

struct lru_cache {
  int size = 0;
  int* arr_count = nullptr;
  int* arr = nullptr;
};


//And numbers of rejection for every type.
int find_element(int* arr, int size_arr, int el) {
  for (int i = 0; i < size_arr; i++) {
    if (arr[i] == el)
      return i;
  }
  
  return -1;
}

int fifo_algo(int* arr_call, int N, int M) {
  //FIFO algorythm
  int cur_last = 0, cur_size = 0, num_fails = 0;
  int* arr = new int[N];

  for (int i = 0; i < N; i++) {
    arr[i] = -1;
  }

  for (int i = 0; i < M; i++) {
    if (find_element(arr, N, arr_call[i]) != -1) {
      if (i != M -1)
        printf("%d+ ", arr_call[i]);
      else 
        printf("%d+", arr_call[i]);
    }
    else {
      if (cur_size < N)
        arr[cur_size++] = arr_call[i];
      else {
        arr[cur_last++] = arr_call[i];
        cur_last = cur_last % N;
      }
      num_fails++;

      if (i != M - 1)
        printf("%d- ", arr_call[i]);
      else
        printf("%d-", arr_call[i]);
    }
  }

  delete[] arr;
  
  return num_fails;
}

int lru_algo(int* arr_call, int N, int M) {
  
  int num_fails = 0;
  
  std::unordered_set<int> set;
  std::unordered_map<int, int> indexes;

  for (int i = 0; i < M; i++) {
    if ((int)set.size() < N) {
      if (set.find(arr_call[i]) == set.end()) {
        set.insert(arr_call[i]);
        num_fails++;
        if (i == M - 1)
          printf("%d-", arr_call[i]);
        else
          printf("%d- ", arr_call[i]);
      }
      else {
        if (i == M - 1)
          printf("%d+", arr_call[i]);
        else
          printf("%d+ ", arr_call[i]);
      }
        

      indexes[arr_call[i]] = i;
    }
    else {
      if (set.find(arr_call[i]) == set.end()) {
        int least_recently_used = INT_MAX, value;

        for (auto j = set.begin(); j != set.end(); j++) {
          if (indexes[*j] < least_recently_used) {
            least_recently_used = indexes[*j];
            value = *j;
          }
        }
        set.erase(value);
        set.insert(arr_call[i]);
        num_fails++;
        if (i != M - 1)
          printf("%d- ", arr_call[i]);
        else 
          printf("%d-", arr_call[i]);
      }
      else {
        if (i != M - 1)
          printf("%d+ ", arr_call[i]);
        else
          printf("%d+", arr_call[i]);
      }
      indexes[arr_call[i]] = i;
    }
  }
  
  return num_fails;
}

int find_min(int* arr_count, int size, int *arr) {
  
  int min_count = arr_count[0];
  int min = 0;

  for (int i = 1; i < size; i++) {
    
    if (arr_count[i] < min_count) {
      min_count = arr_count[i];
      min = i;
    }
    else if (arr_count[i] == min_count && arr[i] < arr[min]) {
      min_count = arr_count[i];
      min = i;
    }
  }

  return min;
}

int lfu_algo(int* arr_call, int N, int M) {

  int* arr_count = new int[N];
  int* arr = new int[N];
  int* arr_last = new int[N];

  int  cur_size = 0, num_fails = 0;
  
  for (int i = 0; i < N; i++) {
    arr[i] = -1;
    arr_count[i] = 0;
    arr_last[i] = 0;
  }

  for (int i = 0; i < M; i++) {
    int pos = find_element(arr, cur_size, arr_call[i]);
    if (pos != -1) {
      
      arr_count[pos]++;
      for (int i = 0; i < cur_size; i++) {
        arr_last[i]++;
      }
      arr_last[pos] = 0;

      if (i != M - 1)
        printf("%d+ ", arr_call[i]);
      else
        printf("%d+", arr_call[i]);
    }
    else {
      if (cur_size < N) {
        arr[cur_size] = arr_call[i];
        arr_count[cur_size]++;
        for (int i = 0; i < cur_size; i++) {
          arr_last[i]++;
        }
        arr_last[cur_size++] = 0;
      }
      else {
        int k = find_min(arr_count, cur_size, arr);
        
        arr_count[k] = 1;
        
        for (int i = 0; i < cur_size; i++) {
          arr_last[i]++;
        }
        arr_last[k] = 0;

        arr[k] = arr_call[i];

      }
      if (i != M - 1)
        printf("%d- ", arr_call[i]);
      else
        printf("%d-", arr_call[i]);
      num_fails++;
    }
  }

  
  delete[] arr_count;
  delete[] arr;
  return num_fails;
}

int find_useless(int* arr, int* arr_call, int N, int M) {
  
  int max = 0, max_count = M+1;
  
  //Считаем количество элементов до певой новой встречи первого элемента из памяти в оставшихся запросах
  for (int j = 0; j < M; j++) {
    if (arr[0] == arr_call[j]) {
      max_count = j;
      max = 0;
      break;
    }
  }
  
  //Считаем для все остальных элементов с 1 по N-1 "время" до следующего вызова, берем самое большое время.
  for (int i = 1; i < N; i++) {
    
    int cur_count = M + 1;
    //Считаем это для других эл
    for (int j = 0; j < M; j++) {
      if (arr[i] == arr_call[j]){
        cur_count = j;
        break;
      }
    }
    
    //Если текущее время больше остальных, то берем его
    if (cur_count > max_count) {
      max = i;
      max_count = cur_count;
    }
    if (max_count == M + 1)
      return max;
  }

  return max;
}

int opt_algo(int* arr_call, int N, int M) {
  int num_fails = 0;
  int* arr = new int[N];
  int cur_size = 0;
  
  for (int i = 0; i < M; i++) {
    
    if (find_element(arr, cur_size, arr_call[i]) != -1) {
      if (i != M - 1)
        printf("%d+ ", arr_call[i]);
      else
        printf("%d+", arr_call[i]);
    }
    else {
      
      if (cur_size < N)
        arr[cur_size++] = arr_call[i];
      else {
        int k = find_useless(arr, arr_call + i + 1, N, M - i);
        arr[k] = arr_call[i];
      }

      num_fails++;

      if (i != M - 1)
        printf("%d- ", arr_call[i]);
      else
        printf("%d-", arr_call[i]);
    }
  }

  delete[] arr;

  return num_fails;
}

int main(void) {
  int M = 0, N = 0;
  
  std::cin >> N >> M;
  
  int* arr_call = new int[M];
  int num_fail[4] = {0};
  

  for (int i = 0; i < M; i++) {
    std::cin >> arr_call[i];
  }
    
  num_fail[0] = fifo_algo(arr_call, N, M);
  printf("\n");
  num_fail[1] = lru_algo(arr_call, N, M);
  printf("\n");
  num_fail[2] = lfu_algo(arr_call, N, M);
  printf("\n");
  num_fail[3] = opt_algo(arr_call, N, M);



  printf("\n%d %d %d %d", num_fail[0], num_fail[1], num_fail[2], num_fail[3]);
  
  delete[] arr_call;

  return 0;
}
