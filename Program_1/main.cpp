#include <algorithm>
#include <arpa/inet.h>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unistd.h>

struct SyncData {
  std::mutex mtx;
  std::condition_variable cv;
  std::string buffer;
  bool ready = false;
};

void process_input(SyncData& syncData) {
  while (true) {
    std::cout << "Введите строку: ";
    std::string input(64, ' ');
    std::cin >> input;

    if (input.length() > 64 ||
        !std::all_of(input.begin(), input.end(), ::isdigit)) {
      std::cout
          << "Строка должна содержать только цифры и не превышать 64 символа."
          << std::endl;
      continue;
    }

    std::sort(input.rbegin(), input.rend());

    for (int i(0); i < input.size(); ++i) {
      auto &c = input[i];
      if ((input[i] - '0') % 2 == 0) {
        c = 'K';
        input.insert(input.rfind(c) + 1, "B");
        ++i;
      }
    }

    std::unique_lock<std::mutex> lock(syncData.mtx);
    syncData.buffer = input;
    syncData.ready = true;
    syncData.cv.notify_one();
  }
}

void process_buffer(SyncData& syncData) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(12345);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  while (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
         0) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  while (true) {
    std::unique_lock<std::mutex> lock(syncData.mtx);
    syncData.cv.wait(lock, [&syncData] { return syncData.ready; });

    std::string data = syncData.buffer;
    syncData.buffer.clear();
    syncData.ready = false;
    lock.unlock();

    std::cout << "Обработанные данные: " << data << std::endl;

    int sum = 0;
    for (char c : data) {
      if (isdigit(c)) {
        sum += c - '0';
      }
    }

    std::string sum_str = std::to_string(sum);
    send(sock, sum_str.c_str(), sum_str.size(), 0);
  }

  close(sock);
}

int main() {
  SyncData syncData;
  std::thread t1(process_input, std::ref(syncData));
  std::thread t2(process_buffer, std::ref(syncData));

  t1.join();
  t2.join();

  return 0;
}
