<?php

$device =  $_REQUEST['device'];
$userkey = $_REQUEST['userkey'];

//$servername = "localhost";
//$username = "reflect_arduino";
//$password = "lozinka12345";
//$dbname = "reflect_arduino";

$servername = "localhost";
$username = "reflect_fuel";
$password = "Lozinka1234";
$dbname = "reflect_fuel";

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);
$conn->query("SET NAMES utf8");

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 


///   kod koji upisuje sve u "log"  

	//	$sql = "INSERT INTO baza_arduino (device, userkey)
	//	VALUES ('".$device ."', '".$userkey ."')";


 

// Create connection
$conn2 = new mysqli($servername, $username, $password, $dbname);
// Check connection
if ($conn2->connect_error) {
    die("Connection failed: " . $conn2->connect_error);
} 

$sql2 = "SELECT * FROM rfid_tags WHERE tag = '".$userkey."' AND enabled = 'yes'";
$result2 = $conn2->query($sql2);

if ($result2->num_rows > 0) {
    // output data of each row
    while($row2 = $result2->fetch_assoc()) {
    echo '#D&';
       // echo "id: " . $row2["tag"]. " <br>";
    }
} else {
    echo '#N&';
}





		// Zatvori konekciju sa bazom nakon inserta
		$conn2->close();
	

	// return all our data to an AJAX call
//	echo json_encode($data);
	