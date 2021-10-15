#include <iostream>


struct pair {
  long long element = 0;
  long long time = 0;
};

//Calculate time for first meeting element in array of calls
long long time_for_first_meating(long long* arr_call, long long M, long long start, long long element) {
  long long time = M;

  for (long long j = start + 1; j < M; j++) {
    if (element == arr_call[j]) {
      time = j - start;
      break;
    }
  }

  return time;
}

//Ищу номер элемента в массиве
//Также уменьшаю время до встречи такого же элемента каждого элемента из кэша
//Пересчитываю время, если элемент встретился
long long find_element(pair* arr, long long N, long long el, long long* arr_call, int M, int start) {
  long long i_ret = -1;
  long long i = 0;

  for (; i < N; i++) {
    if (arr[i].element == el) {
      i_ret = i;
      break;
    }
    arr[i].time--;
  }
  i++;

  for (; i < N; i++)
    arr[i].time--;

  for (i = 0; i < N; i++) {
    if (arr[i].time <= 0)
      arr[i].time = time_for_first_meating(arr_call, M, start, arr[i].element);
  }

  return i_ret;
}

long long max_time(pair* arr, int N) {
  //Теперь ищем максимум по элементам
  long long cur_max = 0;
  
  for (long long j = 0; j < N; j++) {
    if (arr[j].time > arr[cur_max].time)
      cur_max = j;
  }

  return cur_max;
}


unsigned long long opt_algo(long long* arr_call, long long N, long long M) {
  
  unsigned long long  num_fails = 0;
  pair * arr = new pair[N];
  long long  cur_size = 0;

  
  for (long long i = 0; i < M; i++) {
    //Ищу этот элемент в кэше, если не нашлось, то возвращает -1
    long long k = find_element(arr, cur_size, arr_call[i], arr_call, M, i);

    //Если не нашел и кэш уже заполнен
    if (k == -1 && cur_size == N) {
      num_fails++;
      //Считаем время жизни нового элемента

      long long  new_element_time = time_for_first_meating(arr_call, M, i,  arr_call[i]),
        cur_max = max_time(arr, N);
      
      arr[cur_max].element = arr_call[i];
      arr[cur_max].time = new_element_time;
    }
    //Если кэш, еще не заполнен
    else if (k == -1) {
      num_fails++;

      arr[cur_size].element = arr_call[i];
      arr[cur_size].time = 0;
      cur_size++;
      
      //Если кэш заполнен, то считаю время жизни для всех элементов
      if (cur_size == N)
        for (long long k = 0; k < N; k++) {
          //Время жизни с i+1 элемента по M-й, считаем с 1 по N
          arr[k].time = time_for_first_meating(arr_call, M, i, arr[k].element);
        }

    }
  }

  delete[] arr;

  return num_fails;
}

//Как работает алгоритм, я добавляю элемент, не считаю сколько раз он встретится, пока не заполнится весь кэш
//Когда заполнится весь кэш, то при добавлении я подсчитываю время жизни каждого элемента из кэша и время жизни добавляемого элемента
//Выкидываю тот элемент, у которого самое долгое время жизни
//Далее при поиске я уменьшаю время жизни каждого элемента, т.к. элементов остается меньше
//По идеи должно быть оптимальней

//Сложность M * ( find_element + time_for_first_meating + max_time ) = M * (N * M + M + N) = N * M^2
//find_element = N * M
//time_for_first_meating = M 
//max_time = N


//Сложность старого алгоритма:
//M*(find_element + find_useless) = M * (N + M + M * N) = M^2 * N
//fund_element = N
//find_useless = M + N * M

//Выходит, что алгоритм по сложности такой же, как и прежде, нужно улучшать получается константу
int main(void) {
  
  long long N, M;

  std::cin >> N >> M;
  long long * arr_call = new long long[M];
  for (long long i = 0; i < M; i++) {
    std::cin >> arr_call[i];
  }
  printf("%llu", opt_algo(arr_call, N, M));

  return 0;
}
