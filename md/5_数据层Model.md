# 5_数据层Model

db.hpp 中的MySQL封装了MySQL 数据库的基本操作，包括连接、更新和查询，确保了在进行数据库操作时更加方便和安全

\#include "db.h"

\#include <muduo/base/Logging.h>

### 1. `MySQL` 类

- **职责**：提供与 MySQL 数据库的底层接口。负责数据库连接、执行 SQL 查询和更新操作。
- 功能：
  - **连接数据库**：通过 `connect` 方法连接到 MySQL 数据库。
  - **执行更新操作**：通过 `update` 方法执行插入、删除、更新等 SQL 操作。
  - **执行查询操作**：通过 `query` 方法执行 SELECT 查询并返回结果。

```C++
#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>
using namespace std;


// 数据库操作类
class MySQL
{
public:
    // 初始化数据库连接
    MySQL();
    // 释放数据库连接资源
    ~MySQL();
    // 连接数据库
    bool connect();
    // 更新操作
    bool update(string sql);
    // 查询操作
    MYSQL_RES *query(string sql);
    // 获取连接
    MYSQL* getConnection();

private:
    MYSQL *_conn;
};

#endif // !DB_H
```



### 2. `User` 类

- **职责**：表示用户数据的模型。封装了用户的属性，如 ID、用户名、密码和状态。

- 功能

  ：

  - **属性访问**：提供了用户 ID、用户名、密码和状态的 getter 和 setter 方法。
  - **数据表示**：作为数据传输对象（DTO），用于在不同层之间传递用户信息。

user.hpp 中的 User 类负责设置或获取当前 _user 的用户 ID、用户名、密码和状态等数据

```c++
#ifndef USER_H

\#define USER_H



\#include <string>

using namespace std;



class User

{

public:

  User(int id = -1, string name = "", string pwd = "", string state = "offline")

  {

​    this->id = id;

​    this->name = name;

​    this->password = pwd;

​    this->state = state;

  }


  void setId(int id) { this->id = id; }

  void setName(string name) { this->name = name; }

  void setPwd(string pwd) { this->password = pwd; }

  void setState(string state) { this->state = state; }


  //

  int getId() { return this->id; }

  string getName() { return this->name; }

  string getPwd() { return this->password; }

  string getState() { return this->state; }


private:

  int id;

  string name;

  string password;

  string state;

};

\#endif // !USER_H
```



### `UserModel` 类

- **职责**：提供与用户相关的数据库操作功能。利用 `MySQL` 类执行实际的数据库操作，并对 `User` 类进行操作。

- 功能

  ：

  - **插入用户**：通过 `MySQL` 类的接口将 `User` 对象的数据插入到数据库中。
  - **查询用户**：通过 `MySQL` 类的接口从数据库中查询用户数据，并将结果封装到 `User` 对象中。
  - **更新用户状态**：通过 `MySQL` 类的接口更新数据库中用户的状态。

```c++
#ifndef USERMODEL_H

\#define USERMODEL_H


\#include "user.hpp"


// 数据库User表的数据操作类

class UserModel

{

public:

  // User表的增加方法

  bool insert(User &user);


  // 根据用户号码查询用户信息

  User query(int id);


  // 更新用户状态信息

  bool updateState(User user);

};

\#endif // !USERMODEL_H
```



### 关系总结

1. **依赖关系**：
   - `UserModel` 类依赖于 `MySQL` 类，使用 `MySQL` 类提供的接口来执行数据库操作。
   - `UserModel` 类操作的是 `User` 类对象，通过 `User` 类对象传递数据和获取结果。
2. **职责分离**：
   - **`MySQL` 类**：处理与数据库的低级交互细节，如连接、查询、更新等。
   - **`User` 类**：封装用户数据，提供简单的数据表示和访问接口。
   - **`UserModel` 类**：作为业务逻辑层，使用 `MySQL` 类进行数据库操作，操作 `User` 类进行数据管理。
3. **具体使用**：
   - **`UserModel` 类** 在其方法中创建 `MySQL` 类的实例（如 `MySQL mysql;`），并调用其方法来执行实际的数据库操作。
   - **`User` 类** 的实例在 `UserModel` 类中用于存储从数据库中查询到的用户信息，或者将要插入到数据库中的用户信息传递给 `UserModel` 类。