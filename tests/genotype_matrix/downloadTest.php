<?php
namespace Tests\genotype_matrix;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use  Tests\DatabaseSeeders\GenotypeDatasetSeeder;

class downloadTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Tests the Tripal download api integration is intact.
   *
   * @group genotype-matrix
   * @group download
   */
  public function testDownloadAPI() {
    $integrations = nd_genotypes_register_trpdownload_type();
    $this->assertIsArray($integrations,
      "Tripal Download API expects an array of download options supported.");
    $this->assertNotEmpty($integrations,
      "We expect to have some integrations!");

    foreach ($integrations as $details) {
      $this->assertIsArray($details,
        "Each integration details should also be an array.");
      $this->assertNotEmpty($details,
        "Each integration details should not be empyt.");
      $this->assertArrayHasKey('functions', $details,
        "The core part of an integration is it's defined functions. This one doesn't have one...");

      $this->assertArrayHasKey('generate_file', $details['functions'],
        "There is no function defined for generating the file");
      $this->assertTrue(function_exists($details['functions']['generate_file']),
        "The function defined for generating the file does not exist.");

      $this->assertArrayHasKey('get_filename', $details['functions'],
        "There is no function defined for determining the filename.");
      $this->assertTrue(function_exists($details['functions']['get_filename']),
        "The function defined for determining the filename does not exist.");

      $this->assertArrayHasKey('get_file_suffix', $details['functions'],
        "There is no function defined for determining the file suffix.");
      $this->assertTrue(function_exists($details['functions']['get_file_suffix']),
        "The function defined for determining the file sufix does not exist.");
    }
  }

  /**
   * Tests the Tripal download api can determine the file suffix.
   *
   * @group genotype-matrix
   * @group download
   */
  public function testDownloadAPIgetFileSuffix() {
    $vars = [];

    $suffix = trpdownload_genotype_matrix_download_get_csv_suffix($vars);
    $this->assertEquals('csv', $suffix,
      "Unable to determine the correct suffix.");

    $suffix = trpdownload_genotype_matrix_download_get_txt_suffix($vars);
    $this->assertEquals('txt', $suffix,
      "Unable to determine the correct suffix.");
  }

  /**
   * Tests the Tripal download api can determine the file name.
   *
   * @group genotype-matrix
   * @group download
   */
  public function testDownloadAPIgetFileName() {
    $vars = [];
    $vars['download_args']['safe_site_name'] = 'FAKE-SITE-NAME';

    $filename = trpdownload_genotype_matrix_download_get_filename($vars);
    $this->assertNotEquals('', $filename,
      "Ensures the filename is not empty.");
    $this->assertStringContainsString('FAKE-SITE-NAME', $filename,
      "The filename should include the site name.");
  }

  /**
   * Summarizes the download to the user.
   *
   * @group genotype-matrix
   * @group download
   */
  public function testSummarizeDownload() {

    $this->createTables('tripalus');

    // Generate a small dataset.
    $seeder = new GenotypeDatasetSeeder();
    $dataset = $seeder->up(5, TRUE);
    $number_of_variants = count($dataset['data']);
    // @debug print_r($dataset);

    // Compile the arguements.
    $vars = [];
    $vars['download_args']['q'] = [];
    $vars['download_args']['q']['germplasm'][0] = $dataset['germplasm'][0]->stock_id;
    $vars['download_args']['q']['germplasm'][1] = $dataset['germplasm'][1]->stock_id;
    $vars['download_args']['q']['germplasm'][2] = $dataset['germplasm'][2]->stock_id;
    $vars['download_args']['q']['seq_range']['start']['backbone'] = $dataset['chromosome']['record']->name;
    $vars['download_args']['q']['seq_range']['start']['pos'] = 100;
    $vars['download_args']['q']['seq_range']['end']['backbone'] = $dataset['chromosome']['record']->name;
    $vars['download_args']['q']['seq_range']['end']['pos'] = 600;
    $vars['download_args']['q']['sort'] = 'variant_name';
    //$vars['download_args']['q']['polymorphic']['first'] = $dataset['germplasm'][2]->stock_id;
    //$vars['download_args']['q']['polymorphic']['second'] = $dataset['germplasm'][0]->stock_id;
    $vars['download_args']['q']['variant_type'] = 5;
    $vars['download_args']['q']['partition'] = $dataset['organism']->genus;

    $summary = trpdownload_genotype_matrix_download_summarize_download($vars);
    $this->assertNotFalse($summary, "The summary should not be empty.");
    $this->assertStringContainsString('3 germplasm', $summary,
      "We expect to be told there are three germplasm.");
    $this->assertStringContainsString($dataset['chromosome']['record']->name.':100', $summary,
      "We expect to be told thestart coordinate.");
    $this->assertStringContainsString($dataset['chromosome']['record']->name.':600', $summary,
      "We expect to be told thestart coordinate.");
  }

  /**
   * Tests the CSV file can be generated.
   *
   * @group genotype-matrix
   * @group download
   */
  public function testDownloadCsvGenotypes() {

    $this->createTables('tripalus');

    // Generate a small dataset.
    $seeder = new GenotypeDatasetSeeder();
    $dataset = $seeder->up(5, TRUE);
    $number_of_variants = count($dataset['data']);
    // @debug print_r($dataset);

    // Compile the arguements.
    $vars = [];
    $vars['q'] = [];
    $vars['q']['germplasm'][0] = $dataset['germplasm'][0]->stock_id;
    $vars['q']['germplasm'][1] = $dataset['germplasm'][1]->stock_id;
    $vars['q']['germplasm'][2] = $dataset['germplasm'][2]->stock_id;
    $vars['q']['seq_range']['start']['backbone'] = $dataset['chromosome']['record']->name;
    $vars['q']['seq_range']['start']['pos'] = 100;
    $vars['q']['seq_range']['end']['backbone'] = $dataset['chromosome']['record']->name;
    $vars['q']['seq_range']['end']['pos'] = 600;
    $vars['q']['sort'] = 'variant_name';
    //$vars['q']['polymorphic']['first'] = $dataset['germplasm'][2]->stock_id;
    //$vars['q']['polymorphic']['second'] = $dataset['germplasm'][0]->stock_id;
    $vars['q']['variant_type'] = 'SNP';
    $vars['q']['partition'] = $dataset['organism']->genus;
    $vars['site_safe_name'] = 'FAKE-SITE-NAME';
    $vars['type_info'] = [];
    $vars['suffix'] = 'csv';
    $vars['filename'] = 'TEST.download.csv.genotypes.'.date('Ymd').'.csv';
    $vars['format_name'] = 'Comma-separated values';
    $vars['drush'] = FALSE;

    variable_set('trpdownload_fullpath', '/tmp/');
    $directory = variable_get('trpdownload_fullpath', '/tmp/');
    $fullpath = $directory . $vars['filename'];

    // Generate the download file.
    putenv("TRIPAL_SUPPRESS_ERRORS=TRUE");
    ob_start();
    trpdownload_genotype_matrix_download_generate_csv_file($vars, 1);
    $output = ob_get_contents();
    ob_end_clean();
    putenv("TRIPAL_SUPPRESS_ERRORS");

    // Check the output.
    $this->assertStringContainsString("Export Complete (5 rows).", $output,
      "We expect output confirming it's complete with the expected # rows.");

    // Check the file.
    $this->assertDirectoryExists($directory,
      "The download directory doesn't exist.");
    $this->assertFileExists($fullpath,
      "The download file was not created.");
    $file_contents = file_get_contents($fullpath);
    $lines = array_filter(explode("\n",$file_contents));
    $this->assertCount(6, $lines, "We expect the 5 variants plus a header.");
  }

  /**
   * Tests the CSV file can be generated.
   *
   * @group genotype-matrix
   * @group download
   */
  public function testDownloadHapmapGenotypes() {

    $this->createTables('tripalus');

    // Generate a small dataset.
    $seeder = new GenotypeDatasetSeeder();
    $dataset = $seeder->up(5, TRUE);
    $number_of_variants = count($dataset['data']);
    // @debug print_r($dataset);

    // Compile the arguements.
    $vars = [];
    $vars['q'] = [];
    $vars['q']['germplasm'][0] = $dataset['germplasm'][0]->stock_id;
    $vars['q']['germplasm'][1] = $dataset['germplasm'][1]->stock_id;
    $vars['q']['germplasm'][2] = $dataset['germplasm'][2]->stock_id;
    $vars['q']['seq_range']['start']['backbone'] = $dataset['chromosome']['record']->name;
    $vars['q']['seq_range']['start']['pos'] = 100;
    $vars['q']['seq_range']['end']['backbone'] = $dataset['chromosome']['record']->name;
    $vars['q']['seq_range']['end']['pos'] = 600;
    $vars['q']['sort'] = 'variant_name';
    //$vars['q']['polymorphic']['first'] = $dataset['germplasm'][2]->stock_id;
    //$vars['q']['polymorphic']['second'] = $dataset['germplasm'][0]->stock_id;
    $vars['q']['variant_type'] = 'SNP';
    $vars['q']['partition'] = $dataset['organism']->genus;
    $vars['site_safe_name'] = 'FAKE-SITE-NAME';
    $vars['type_info'] = [];
    $vars['suffix'] = 'txt';
    $vars['filename'] = 'TEST.download.hapmap.genotypes.'.date('Ymd').'.txt';
    $vars['format_name'] = 'Haplotype mapping';
    $vars['drush'] = FALSE;

    variable_set('trpdownload_fullpath', '/tmp/');
    $directory = variable_get('trpdownload_fullpath', '/tmp/');
    $fullpath = $directory . $vars['filename'];

    // Generate the download file.
    putenv("TRIPAL_SUPPRESS_ERRORS=TRUE");
    ob_start();
    trpdownload_genotype_matrix_download_generate_hapmap_file($vars, 1);
    $output = ob_get_contents();
    ob_end_clean();
    putenv("TRIPAL_SUPPRESS_ERRORS");

    // Check the output.
    $this->assertStringContainsString("Export Complete (5 rows).", $output,
      "We expect output confirming it's complete with the expected # rows.");

    // Check the file.
    $this->assertDirectoryExists($directory,
      "The download directory doesn't exist.");
    $this->assertFileExists($fullpath,
      "The download file was not created.");
    $file_contents = file_get_contents($fullpath);
    $lines = array_filter(explode("\n",$file_contents));
    $this->assertCount(6, $lines, "We expect the 5 variants plus a header.");
  }

  /**
   * HELPER: Create tables needed for mviews.
   *
   * @param $partition
   *   The name of the partition to create tables for (lowercase).
   */
  public function createTables($partition) {
    $calls_table = 'mview_ndg_'.$partition.'_calls';
    nd_genotypes_create_mview_ndg_calls($calls_table);
    $variant_table = 'mview_ndg_'.$partition.'_variants';
    nd_genotypes_create_mview_ndg_variants($variant_table);
    $germplasm_table = 'mview_ndg_germplasm_genotyped';
    nd_genotypes_create_mview_ndg_germplasm_genotyped($germplasm_table);
  }
}
