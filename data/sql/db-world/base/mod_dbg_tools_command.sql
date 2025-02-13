-- COMMAND
DELETE FROM `command` WHERE `name` IN ('dbg', 'dbg reset dailies');
INSERT INTO `command` (`name`, `security`, `help`) VALUES
('dbg', 0, 'Syntax: .dbg'),
('dbg reset', 0, 'Syntax: .dbg reset'),
('dbg reset dailies', 0, 'Syntax: .dbg reset dailies\nReset daily quests for self.');