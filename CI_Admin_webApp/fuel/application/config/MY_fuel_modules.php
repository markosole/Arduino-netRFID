<?php 
/*
|--------------------------------------------------------------------------
| MY Custom Modules
|--------------------------------------------------------------------------
|
| Specifies the module controller (key) and the name (value) for fuel
*/


/*********************** EXAMPLE ***********************************

$config['modules']['quotes'] = array(
	'preview_path' => 'about/what-they-say',
);

$config['modules']['projects'] = array(
	'preview_path' => 'showcase/project/{slug}',
	'sanitize_images' => FALSE // to prevent false positives with xss_clean image sanitation
);

*********************** /EXAMPLE ***********************************/

//$config['modules']['rfid'] = array(
 //   'module_name' => 'RFID',
 //   'module_uri' => 'rfid',
 //   'model_name' => 'rfid_model',
//    'model_location' => '',
 //   'display_field' => 'name',
//    'preview_path' => '',
 //   'permission' => 'rfid',
 //   'instructions' => 'Here you can manage the RFID users.',
//    'archivable' => TRUE,
//    'nav_selected' => 'rfidkartice'
//);

//$config['modules']['authors'] = array();
//$config['modules']['articles'] = array();

$config['modules']['rfid'] = array(
    'module_name' => 'RFID Tags',
    'module_uri' => 'rfid',
    'model_name' => 'rfid_model',
    'model_location' => '',
    'display_field' => 'name',
    'preview_path' => '',
    'permission' => 'rfid',
    'instructions' => 'Here you can manage RFID Tags.',
    'archivable' => TRUE,
    'nav_selected' => 'rfid'
);
$config['modules']['categories'] = array(
	'module_name' => 'Card readers',
);


/*********************** OVERWRITES ************************************/

$config['module_overwrites']['categories']['hidden'] = TRUE; // change to FALSE if you want to use the generic categories module
$config['module_overwrites']['tags']['hidden'] = TRUE; // change to FALSE if you want to use the generic tags module

/*********************** /OVERWRITES ************************************/