#include "storage.h"
#include "user.h"
#include "book_data.h"
#include "log.h"


int main() {
  try {
    // 初始化 LoginStatus 实例
    LoginStatus login_status;

    // 1. 测试注册帐户
    std::cout << "注册帐户: testuser1\n";
    login_status.reg("testuser1", "password123", "Test User 1");

    // 2. 测试登录帐户
    std::cout << "登录帐户: testuser1\n";
    login_status.su("testuser1", "password123");

    // 3. 测试获取当前帐户的权限
    std::cout << "当前帐户权限: " << login_status.get_privilege() << "\n";  // 应该是 1

    // 4. 测试修改密码
    std::cout << "修改密码: testuser1\n";
    login_status.passwd("testuser1", "newpassword456", "password123");

    // 5. 测试注销帐户
    std::cout << "注销帐户\n";
    login_status.logout();

    // 6. 测试创建新帐户
    std::cout << "创建帐户: adminuser\n";
    login_status.useradd("adminuser", "adminpassword", 7, "Admin User");

    // 7. 测试删除帐户
    std::cout << "删除帐户: testuser1\n";
    login_status.erase("testuser1");

    std::cout << "所有操作完成。\n";
  }
  catch (const invalid_command& e) {
    std::cerr << "发生无效操作错误！\n";
  }
  catch (const std::exception& e) {
    std::cerr << "发生错误: " << e.what() << "\n";
  }

}