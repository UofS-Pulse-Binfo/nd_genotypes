
Features
========

- Extensive configuration allowing for flexiblity in ontology terms used, as well as, colours and wording used in visualizations.
- Multiple Tripal 3 Fields which provide flexible, configurable summaries of genotypic data.
    - Marker/Variant Genotype Summary: a pie chart showing the ratio of alleles recorded per marker.
    - Marker/Variant Flanking Sequence: a FASTA record showing flanking sequence with all known variants indicated via IUPAC codes (useful in marker design).
    - Marker List: provides links to the markers assaying a given variant.
    - Project Genotype Summary (comming soon): provides a summary of all genotypic data associated with a given project.
    - Germplasm Genotype Summary (comming soon): provides a summary of all genotypic data collected from a given germplasm.
    - Genotype Matrix Quick Search (comming soon): provides a quick search for marker, variant, project and germplasm pages which redirects to a filtered genotype matrix.
- Genotype Matrix search allowing users to extract genotypes for a user-defined set of germplasm. Includes filtering by marker/variant type, variant location, and pairwise polymorphism. Filtering by quality is comming soon.
- Integration of all fields with Tripal 3 web services allowing you to share your genotypic data with other groups.
  
.. toctree::
   :maxdepth: 2
   
   features/genotype_matrix
   features/feature_genotype_summaries
   features/sequence_with_variants_field
