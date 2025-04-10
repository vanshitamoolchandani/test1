import json
from docx import Document
from docx.oxml.ns import qn

def extract_points_from_docx(docx_filename, target_headings):
    """
    Extract numbered main points and sub-points from sections under target headings.
    
    Parameters:
      docx_filename (str): Path to the DOCX file.
      target_headings (list of str): List of headings for which to extract numbered points.
    
    Returns:
      dict: A dictionary with target heading as keys, and each value is a dictionary containing:
            - "point_count": Number of main (level 0) points.
            - "points": A list of dictionaries, each with "point_text" and "sub_point_count".
    """
    def get_numbering_level(para):
        """
        Returns the numbering level (an integer) for a paragraph if it has Word numbering.
        Otherwise, returns None.
        """
        p = para._p  # Access underlying XML element
        if p.pPr is not None and p.pPr.numPr is not None:
            ilvl = p.pPr.numPr.find(qn("w:ilvl"))
            if ilvl is not None:
                level = ilvl.get(qn("w:val"))
                if level is not None:
                    return int(level)
        return None

    def process_section(paragraphs):
        """
        Process a list of paragraphs from a section by counting main points (level 0)
        and sub points (level 1 or greater) for each main point.
        """
        section_data = {"point_count": 0, "points": []}
        current_point = None

        for para in paragraphs:
            level = get_numbering_level(para)
            if level is not None:
                if level == 0:
                    # Finalize the previous main point if exists.
                    if current_point is not None:
                        section_data["points"].append(current_point)
                    current_point = {
                        "point_text": para.text.strip(),
                        "sub_point_count": 0
                    }
                    section_data["point_count"] += 1
                else:
                    # For sub points (level > 0), increment sub_point_count.
                    if current_point is not None:
                        current_point["sub_point_count"] += 1

        if current_point is not None:
            section_data["points"].append(current_point)

        return section_data

    # Open the document.
    doc = Document(docx_filename)
    results = {}
    
    current_section_heading = None
    current_section_paragraphs = []  # To store paragraphs of the active section.

    for para in doc.paragraphs:
        # Check if the paragraph is a heading (assuming Word heading styles).
        if para.style.name.startswith("Heading"):
            # If we are already processing a target section, finalize it when a new heading is encountered.
            if current_section_heading is not None:
                results[current_section_heading] = process_section(current_section_paragraphs)
                current_section_heading = None
                current_section_paragraphs = []
            # If the heading text is in our target list, start capturing its section.
            if para.text.strip() in target_headings:
                current_section_heading = para.text.strip()
        else:
            # If inside a target section, add paragraphs to current section.
            if current_section_heading is not None:
                current_section_paragraphs.append(para)
    
    # Finalize the last section if it's still active.
    if current_section_heading is not None:
        results[current_section_heading] = process_section(current_section_paragraphs)
    
    return results

# Example usage:
if __name__ == "__main__":
    # Define the headings you want to extract.
    target_headings = ["Inclusion Criteria", "Exclusion criteria"]
    # Replace 'your_file.docx' with the path to your DOCX.
    data = extract_points_from_docx("your_file.docx", target_headings)
    # Print the output in JSON format.
    print(json.dumps(data, indent=2))
