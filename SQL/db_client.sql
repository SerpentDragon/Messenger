DROP TABLE IF EXISTS Personal CASCADE;
CREATE TABLE Personal 
(
    id INT PRIMARY KEY,
    nickname VARCHAR(50) UNIQUE NOT NULL,
	private_key TEXT NOT NULL,
	public_key TEXT NOT NULL,
	picture VARCHAR(200)
);

DROP TABLE IF EXISTS Contact CASCADE;
CREATE TABLE Contact
(
	contact_id INT,
	nickname VARCHAR(50) NOT NULL,
	public_key TEXT NOT NULL,
	picture VARCHAR(200),
	personal_id INT NOT NULL,
	PRIMARY KEY (contact_id, personal_id),
	FOREIGN KEY (personal_id) REFERENCES Personal(id)
);

DROP TABLE IF EXISTS Chat CASCADE;
CREATE TABLE Chat
(
	chat_id INT NOT NULL,
	name VARCHAR(50) NOT NULL,
	picture VARCHAR(200),
	personal_id INT NOT NULL,
	PRIMARY KEY (chat_id, personal_id),
	FOREIGN KEY (personal_id) REFERENCES Personal(id)
);

DROP TABLE IF EXISTS ChatParticipants;
CREATE TABLE ChatParticipants
(
	contact_id INT,
	chat_id INT,
	personal_id INT,
	PRIMARY KEY (contact_id, chat_id, personal_id),
	FOREIGN KEY (contact_id, personal_id) REFERENCES Contact(contact_id, personal_id),
	FOREIGN KEY (chat_id, personal_id) REFERENCES Chat(chat_id, personal_id)
);

DROP TABLE IF EXISTS Message;
CREATE TABLE Message
(
	id SERIAL PRIMARY KEY,
	receiver INT NOT NULL,
	sender INT NOT NULL,
	text VARCHAR(2000) NOT NULL,
	timestamp BIGINT NOT NULL,
	chat_id INT,
	vanishing BOOL,
	personal_id INT NOT NULL,
	FOREIGN KEY (chat_id, personal_id) REFERENCES Chat(chat_id, personal_id),
	FOREIGN KEY (personal_id) REFERENCES Personal(id)
);
