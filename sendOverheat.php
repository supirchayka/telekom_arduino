<?php
$host = 'localhost';
$db = 'house';
$user = 'root';
$password = '12345678';

$dsn = "mysql:host=$host;dbname=$db;charset=UTF8";

try {
    $pdo = new PDO($dsn, $user, $password);

    if ($pdo) {
        echo "Connected to the $db database successfully!";
    }

    sendTemperatures($pdo, $_POST);

} catch (PDOException $exception) {
    echo $exception->getMessage();
}

function sendTemperatures($pdo, $data)
{
    try {
        $sql = 'INSERT INTO temperature(date, state, description, temperatures) VALUES(:date, :state, :description, :temperatures)';

        $statement = $pdo->prepare($sql);

        $statement->execute([
            ':date' => date('Y-m-d H:i:s'),
            ':state' => $data['state'],
            ':description' => $data['description'],
            ':temperatures' => $data['temperatures']
        ]);
    } catch (PDOException $exception) {
        echo $exception->getMessage();
    }

}
