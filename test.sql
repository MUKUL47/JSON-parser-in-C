CREATE TABLE users (id INT, name TEXT, age INT);
UPDATE users SET age = 26 WHERE id >= 1 OR name = 'Alice';