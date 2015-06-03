namespace cpp test

exception NotFound {
	1: i32 id,
}

struct User {
	1: i32 id,
	2: string name,
	3: string email,
}

service UserManager {
	list<User> all(),
	User get(1:i32 id) throws(1: NotFound err),
}
