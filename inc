from docx import Document

def extract_criteria_sections(file_path):
    """
    Extracts the Inclusion and Exclusion criteria sections from a Word document.
    
    Parameters:
        file_path (str): Path to the .docx file.
        
    Returns:
        dict: A dictionary with 'inclusion' and 'exclusion' keys containing the respective sections' text.
        
    Raises:
        ValueError: If the required headings are not found in the expected order.
    """
    doc = Document(file_path)
    
    # Initialize variables to track positions of headings
    inclusion_pos = None
    exclusion_pos = None
    lifestyle_pos = None
    
    # Find positions of required headings
    for i, para in enumerate(doc.paragraphs):
        if para.style.name.startswith('Heading'):
            heading_text = para.text.strip()
            if heading_text == "Inclusion criteria":
                inclusion_pos = i
            elif heading_text == "Exclusion criteria" and inclusion_pos is not None:
                exclusion_pos = i
            elif heading_text == "Lifestyle considerations" and exclusion_pos is not None:
                lifestyle_pos = i
                break  # No need to continue after finding lifestyle
    
    # Validate all headings were found
    if None in (inclusion_pos, exclusion_pos, lifestyle_pos):
        missing = []
        if inclusion_pos is None: missing.append("Inclusion criteria")
        if exclusion_pos is None: missing.append("Exclusion criteria")
        if lifestyle_pos is None: missing.append("Lifestyle considerations")
        raise ValueError(f"Missing headings: {', '.join(missing)}")
    
    # Validate heading order
    if not (inclusion_pos < exclusion_pos < lifestyle_pos):
        raise ValueError("Headings are not in the expected order")
    
    # Extract content between headings
    def get_section_text(start, end):
        return "\n".join(
            para.text.strip() 
            for para in doc.paragraphs[start+1:end]
            if para.text.strip()
        )
    
    return {
        'inclusion': get_section_text(inclusion_pos, exclusion_pos),
        'exclusion': get_section_text(exclusion_pos, lifestyle_pos)
    }
