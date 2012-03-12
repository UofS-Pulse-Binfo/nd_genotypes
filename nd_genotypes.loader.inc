<?php

/**
 * @file
 * Functions in this file will aid in loading genotypes and their experiments
 * into the appropriate tables. 
 *
 * A bulk loading template will be created for loading 
 * generic Marker => Genotype for Stock mappings
 */

/** 
 * Create a Generic Genotype Experiment Bulk Loader and save it to the
 * tripal_bulk_loader_template table for use in Bulk Loading Jobs
 *
 * Assumes a tsv file with the following columns:
 *  - feature.uniquename: the marker the genotypes should be associated with (must already exist)
 *  - genotype.description: the actual genotype called (ie: AA, 300bp, present, etc.)
 *
 * The following fields are constants per bulk loading job and will be able to be entered 
 * once the job is created (* -required):
 * ( currently the template needs to be edited )
 *  - stock.uniquename*: the stock the genotypes were called in (must already exist)
 *  - genotype column*: column the genotypes are in (index starts at 1)
 *  - project.uniquename*: the project to associate the genotypes with (must already exist)
 *  - nd_geolocation.description*: the location the genotyping experiment took place at (must already exist)
 *
 * This loader will create the following records (number of records per line in the file):
 *  - nd_experiment (1)
 *  - nd_experiment_genotype (1)
 *  - genotype record (0 or 1 -depending on if it already exists for that feature)
 *  - feature_genotype (0 or 1 -depending on if it already exists for that feature)
 *  - nd_experment_project (1)
 *  - nd_experiment_stock (1)
 *
 * @return
 *   template_id on success (both insert and update) and FALSE otherwise
 */
function nd_genotypes_add_generic_experiment_bulk_loader ($update = FALSE) {

	$template_name = 'ND Genotype: Generic Genotype Experiments';
	$serialized_template = 'a:16:{i:0;a:4:{s:5:"table";s:7:"feature";s:9:"record_id";s:16:"Feature (Marker)";s:6:"fields";a:1:{i:0;a:7:{s:4:"type";s:11:"table field";s:5:"title";s:10:"Uniquename";s:5:"field";s:10:"uniquename";s:8:"required";i:0;s:17:"spreadsheet sheet";s:6:"Sheet1";s:18:"spreadsheet column";s:1:"1";s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}}s:4:"mode";s:6:"select";}i:1;a:3:{s:5:"table";s:8:"genotype";s:9:"record_id";s:8:"Genotype";s:6:"fields";a:2:{i:0;a:11:{s:4:"type";s:11:"table field";s:5:"title";s:19:"Genotype Uniquename";s:5:"field";s:10:"uniquename";s:8:"required";i:0;s:17:"spreadsheet sheet";s:6:"Sheet1";s:18:"spreadsheet column";s:1:"3";s:5:"regex";a:2:{s:7:"pattern";a:1:{i:0;s:8:"/^(.*)$/";}s:7:"replace";a:1:{i:0;s:15:"<#column:1#>_\1";}}s:8:"priority";s:1:"1";s:11:"field_index";s:1:"0";s:7:"exposed";i:1;s:19:"exposed_description";s:116:"Genotype Column (contains allele call) -Marker name will be prefixed to the genotype to form the genotype uniquename";}i:1;a:11:{s:4:"type";s:11:"table field";s:5:"title";s:6:"Allele";s:5:"field";s:11:"description";s:8:"required";i:0;s:17:"spreadsheet sheet";s:6:"Sheet1";s:18:"spreadsheet column";s:1:"3";s:5:"regex";a:1:{s:7:"pattern";a:0:{}}s:8:"priority";s:1:"1";s:11:"field_index";s:1:"1";s:7:"exposed";i:1;s:19:"exposed_description";s:38:"Genotype Column (contains allele call)";}}}i:2;a:4:{s:5:"table";s:2:"cv";s:9:"record_id";s:12:"CV -sequence";s:6:"fields";a:1:{i:0;a:6:{s:4:"type";s:8:"constant";s:5:"title";s:4:"Name";s:5:"field";s:4:"name";s:8:"required";i:0;s:14:"constant value";s:8:"sequence";s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}}s:4:"mode";s:6:"select";}i:3;a:4:{s:5:"table";s:6:"cvterm";s:9:"record_id";s:18:"CVterm -variant_of";s:6:"fields";a:2:{i:0;a:6:{s:4:"type";s:8:"constant";s:5:"title";s:4:"Name";s:5:"field";s:4:"name";s:8:"required";i:0;s:14:"constant value";s:10:"variant_of";s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}i:1;a:6:{s:4:"type";s:11:"foreign key";s:5:"title";s:2:"CV";s:5:"field";s:5:"cv_id";s:11:"foreign key";s:12:"CV -sequence";s:8:"required";i:0;s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}}s:4:"mode";s:6:"select";}i:4;a:3:{s:5:"table";s:16:"feature_genotype";s:9:"record_id";s:19:"Feature => Genotype";s:6:"fields";a:5:{i:0;a:6:{s:4:"type";s:11:"foreign key";s:5:"title";s:7:"Feature";s:5:"field";s:10:"feature_id";s:11:"foreign key";s:16:"Feature (Marker)";s:8:"required";i:0;s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}i:1;a:6:{s:4:"type";s:11:"foreign key";s:5:"title";s:8:"Genotype";s:5:"field";s:11:"genotype_id";s:11:"foreign key";s:8:"Genotype";s:8:"required";i:0;s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}i:2;a:6:{s:4:"type";s:8:"constant";s:5:"title";s:4:"Rank";s:5:"field";s:4:"rank";s:8:"required";i:0;s:14:"constant value";s:1:"0";s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}i:3;a:6:{s:4:"type";s:8:"constant";s:5:"title";s:6:"cgroup";s:5:"field";s:6:"cgroup";s:8:"required";i:0;s:14:"constant value";s:1:"0";s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}i:4;a:6:{s:4:"type";s:11:"foreign key";s:5:"title";s:6:"cvterm";s:5:"field";s:9:"cvterm_id";s:11:"foreign key";s:18:"CVterm -variant_of";s:8:"required";i:0;s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}}}i:5;a:4:{s:5:"table";s:2:"cv";s:9:"record_id";s:21:"CV -Natural Diversity";s:6:"fields";a:1:{i:0;a:8:{s:4:"type";s:8:"constant";s:5:"title";s:4:"Name";s:5:"field";s:4:"name";s:8:"required";i:0;s:14:"constant value";s:17:"natural_diversity";s:5:"regex";a:1:{s:7:"pattern";a:0:{}}s:8:"priority";s:1:"5";s:11:"field_index";s:1:"0";}}s:4:"mode";s:6:"select";}i:6;a:4:{s:5:"table";s:6:"cvterm";s:9:"record_id";s:18:"ND Experiment Type";s:6:"fields";a:2:{i:0;a:6:{s:4:"type";s:8:"constant";s:5:"title";s:4:"Name";s:5:"field";s:4:"name";s:8:"required";i:0;s:14:"constant value";s:14:"genotype_assay";s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}i:1;a:6:{s:4:"type";s:11:"foreign key";s:5:"title";s:2:"CV";s:5:"field";s:5:"cv_id";s:11:"foreign key";s:21:"CV -Natural Diversity";s:8:"required";i:0;s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}}s:4:"mode";s:6:"select";}i:7;a:4:{s:5:"table";s:14:"nd_geolocation";s:9:"record_id";s:14:"ND Geolocation";s:6:"fields";a:1:{i:0;a:10:{s:4:"type";s:8:"constant";s:5:"title";s:23:"Geolocation Description";s:5:"field";s:11:"description";s:8:"required";i:0;s:14:"constant value";s:0:"";s:5:"regex";a:1:{s:7:"pattern";a:0:{}}s:8:"priority";s:1:"7";s:11:"field_index";s:1:"0";s:16:"exposed_validate";i:1;s:7:"exposed";i:1;}}s:4:"mode";s:6:"select";}i:8;a:4:{s:5:"table";s:13:"nd_experiment";s:9:"record_id";s:13:"ND Experiment";s:6:"fields";a:2:{i:0;a:6:{s:4:"type";s:11:"foreign key";s:5:"title";s:4:"Type";s:5:"field";s:7:"type_id";s:11:"foreign key";s:18:"ND Experiment Type";s:8:"required";i:0;s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}i:1;a:6:{s:4:"type";s:11:"foreign key";s:5:"title";s:8:"Location";s:5:"field";s:17:"nd_geolocation_id";s:11:"foreign key";s:14:"ND Geolocation";s:8:"required";i:0;s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}}s:4:"mode";s:6:"insert";}i:9;a:4:{s:5:"table";s:22:"nd_experiment_genotype";s:9:"record_id";s:25:"ND Experiment => Genotype";s:6:"fields";a:2:{i:0;a:6:{s:4:"type";s:11:"foreign key";s:5:"title";s:13:"ND Experiment";s:5:"field";s:16:"nd_experiment_id";s:11:"foreign key";s:13:"ND Experiment";s:8:"required";i:0;s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}i:1;a:6:{s:4:"type";s:11:"foreign key";s:5:"title";s:8:"Genotype";s:5:"field";s:11:"genotype_id";s:11:"foreign key";s:8:"Genotype";s:8:"required";i:0;s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}}s:4:"mode";s:6:"insert";}i:10;a:4:{s:5:"table";s:7:"project";s:9:"record_id";s:7:"Project";s:6:"fields";a:1:{i:0;a:10:{s:4:"type";s:8:"constant";s:5:"title";s:12:"Project Name";s:5:"field";s:4:"name";s:8:"required";i:0;s:14:"constant value";s:0:"";s:5:"regex";a:1:{s:7:"pattern";a:0:{}}s:8:"priority";s:2:"10";s:11:"field_index";s:1:"0";s:16:"exposed_validate";i:1;s:7:"exposed";i:1;}}s:4:"mode";s:6:"select";}i:11;a:4:{s:5:"table";s:21:"nd_experiment_project";s:9:"record_id";s:24:"ND Experiment => Project";s:6:"fields";a:2:{i:0;a:6:{s:4:"type";s:11:"foreign key";s:5:"title";s:13:"ND Experiment";s:5:"field";s:16:"nd_experiment_id";s:11:"foreign key";s:13:"ND Experiment";s:8:"required";i:0;s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}i:1;a:6:{s:4:"type";s:11:"foreign key";s:5:"title";s:7:"Project";s:5:"field";s:10:"project_id";s:11:"foreign key";s:7:"Project";s:8:"required";i:0;s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}}s:4:"mode";s:6:"insert";}i:12;a:4:{s:5:"table";s:5:"stock";s:9:"record_id";s:5:"Stock";s:6:"fields";a:1:{i:0;a:10:{s:4:"type";s:8:"constant";s:5:"title";s:16:"Stock Uniquename";s:5:"field";s:10:"uniquename";s:8:"required";i:0;s:14:"constant value";s:0:"";s:5:"regex";a:1:{s:7:"pattern";a:0:{}}s:8:"priority";s:2:"12";s:11:"field_index";s:1:"0";s:16:"exposed_validate";i:1;s:7:"exposed";i:1;}}s:4:"mode";s:6:"select";}i:13;a:4:{s:5:"table";s:2:"cv";s:9:"record_id";s:16:"CV -Relationship";s:6:"fields";a:1:{i:0;a:6:{s:4:"type";s:8:"constant";s:5:"title";s:4:"Name";s:5:"field";s:4:"name";s:8:"required";i:0;s:14:"constant value";s:12:"relationship";s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}}s:4:"mode";s:6:"select";}i:14;a:4:{s:5:"table";s:6:"cvterm";s:9:"record_id";s:21:"Stock Connection Type";s:6:"fields";a:2:{i:0;a:6:{s:4:"type";s:8:"constant";s:5:"title";s:4:"Name";s:5:"field";s:4:"name";s:8:"required";i:0;s:14:"constant value";s:15:"participates_in";s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}i:1;a:6:{s:4:"type";s:11:"foreign key";s:5:"title";s:2:"CV";s:5:"field";s:5:"cv_id";s:11:"foreign key";s:16:"CV -Relationship";s:8:"required";i:0;s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}}s:4:"mode";s:6:"select";}i:15;a:4:{s:5:"table";s:19:"nd_experiment_stock";s:9:"record_id";s:22:"ND Experiment => Stock";s:6:"fields";a:3:{i:0;a:6:{s:4:"type";s:11:"foreign key";s:5:"title";s:13:"ND Experiment";s:5:"field";s:16:"nd_experiment_id";s:11:"foreign key";s:13:"ND Experiment";s:8:"required";i:0;s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}i:1;a:6:{s:4:"type";s:11:"foreign key";s:5:"title";s:5:"Stock";s:5:"field";s:8:"stock_id";s:11:"foreign key";s:5:"Stock";s:8:"required";i:0;s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}i:2;a:6:{s:4:"type";s:11:"foreign key";s:5:"title";s:15:"Connection Type";s:5:"field";s:7:"type_id";s:11:"foreign key";s:21:"Stock Connection Type";s:8:"required";i:0;s:5:"regex";a:1:{s:7:"pattern";a:0:{}}}}s:4:"mode";s:6:"insert";}}';
	$record = array(
		'name' => $template_name,
		'template_array' => $serialized_template
	);
	
	// Check if it already exists
	$exists = db_fetch_object(db_query("SELECT * FROM {tripal_bulk_loader_template} WHERE name='%s'",$record['name']));
	if ($exists->template_id) {
		$record['template_id'] = $exists->template_id;
		if ($update) {
			// Update the existing template
			$success = drupal_write_record('tripal_bulk_loader_template',$record,'template_id');
			if ($success) {
				drupal_set_message('ND Genotype: Generic Genotype Experiments Bulk Loader Template Updated');
				return $exists->template_id;
			} else {
				drupal_set_message('Unable to update ND Genotype: Generic Genotype Experiments Bulk Loader Template','error');
				return FALSE;
			}			
		} else {
			drupal_set_message('ND Genotype: Generic Genotype Experiments Bulk Loader Template already exists.','warning');
			return $exists->template_id;
		}
		
		
	// if it doesn't already exist then insert it
	} else {
		$success = drupal_write_record('tripal_bulk_loader_template',$record);
		if ($success) {
			drupal_set_message('ND Genotype: Generic Genotype Experiments Bulk Loader Template Created');
			return $success['template_id'];
		} else {
			drupal_set_message('Unable to create ND Genotype: Generic Genotype Experiments Bulk Loader Template','error');
			return FALSE;
		}
	}
}