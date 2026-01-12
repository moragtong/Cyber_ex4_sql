<?php
// Database credentials - takes them from the environment variables we define when uploading the container
$host = getenv('DB_HOST');
$username = getenv('DB_UNAME'); 
$password = getenv('DB_PASSWORD'); 
$database = getenv('DB_NAME'); 

//remove error reporting
error_reporting(0);

// Establish connection
$conn = new mysqli($host, $username, $password, $database);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Initialize variables
$order_id_to_check = null;
$message = "";

// Check if form is submitted
if ($_SERVER['REQUEST_METHOD'] === 'GET' && isset($_GET['order_id'])) { 
    // The porpuse of  isset($_GET['order_id'] is to make sure the cde does not run with undefined variable.
    $order_id_to_check = $_GET['order_id'];

    $sql = "SELECT is_sent FROM orders WHERE order_id = $order_id_to_check";
    $result = $conn->query($sql);

    if ($result->num_rows > 0) {
        $row = $result->fetch_assoc();
        $is_sent = $row['is_sent'];
        $message = $is_sent ? "Your order has been sent!" : "Your order has not been sent yet.";
    } 
    else {
        $message = "Your order has not been sent yet.";
    }
}

// Close the connection
$conn->close();
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Vulnerable Order Check</title>
</head>
<body>
    <h1>Check Your Order Status</h1>
    <form method="GET" action="">
        <label for="order_id">Enter Your Order ID:</label>
        <input type="text" name="order_id" id="order_id" required>
        <button type="submit">Check Status</button>
    </form>
    <?php if ($message): ?>
        <div class="result"><?php echo htmlspecialchars($message); ?></div>
    <?php endif; ?>
</body>
</html>
