public static void main(String[] args) {
    LOGGER.info("Starting PDF export operation");
    
    try (InputStream inputStream = Files.newInputStream(new File("src/main/resources/Bodea Brochure.pdf").toPath())) {
        LOGGER.debug("Input stream obtained successfully");
        
        // Initial setup, create credentials instance
        LOGGER.info("Creating credentials");
        Credentials credentials = new ServicePrincipalCredentials(
                System.getenv("PDF_SERVICES_CLIENT_ID"),
                System.getenv("PDF_SERVICES_CLIENT_SECRET"));
        
        LOGGER.info("Initializing PDFServices client");
        PDFServices pdfServices = new PDFServices(credentials);
        
        LOGGER.info("Uploading asset");
        Asset asset = pdfServices.upload(inputStream, PDFServicesMediaType.PDF.getMediaType());
        
        LOGGER.info("Creating export parameters");
        ExportPDFParams exportPDFParams = ExportPDFParams.exportPDFParamsBuilder(ExportPDFTargetFormat.DOCX)
                .build();
        
        LOGGER.info("Creating export job");
        ExportPDFJob exportPDFJob = new ExportPDFJob(asset, exportPDFParams);
        
        LOGGER.info("Submitting job");
        String location = pdfServices.submit(exportPDFJob);
        LOGGER.debug("Job location: {}", location);
        
        LOGGER.info("Getting job result");
        PDFServicesResponse<ExportPDFResult> pdfServicesResponse = pdfServices.getJobResult(location, ExportPDFResult.class);
        
        LOGGER.info("Processing result");
        Asset resultAsset = pdfServicesResponse.getResult().getAsset();
        StreamAsset streamAsset = pdfServices.getContent(resultAsset);
        
        Files.createDirectories(Paths.get("output/"));
        OutputStream outputStream = Files.newOutputStream(new File("output/Bodea Brochure.docx").toPath());
        LOGGER.info("Saving output file");
        IOUtils.copy(streamAsset.getInputStream(), outputStream);
        
    } catch (Exception e) { // Catch all exceptions
        LOGGER.error("Operation failed: {}", e.getMessage(), e);
    }
    LOGGER.info("Operation completed");
}
