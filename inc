def extract_criteria(file_path):
    doc = Document(file_path)
    print("DOCX file loaded. Starting processing.")

    # Combine paragraphs into a single text string; alternatively, iterate paragraph by paragraph.
    text = "\n".join(paragraph.text for paragraph in doc.paragraphs)
    
    criteria = {}
    current_section = None
    collecting = False
    for i, para in enumerate(doc.paragraphs):
        line = para.text.strip()
        style = para.style.name if para.style else ""
        # print(f"Processing paragraph: '{line}' with style: '{style}'")
        
        # Skip empty paragraphs.
        if not line:
            continue
        
        # Only treat the paragraph as a heading if its style starts with "Heading"
        if style.startswith("Heading"):
            # Check for stopping condition.
            if "Lifestyle Considerations" in line:
                print(f"Encountered 'Lifestyle considerations' heading. Stopping parsing. {line} {i}\n")
                break
            
            # Switch section if the heading text is matched.
            if "Inclusion Criteria" in line:
                current_section = 'Inclusion_Criteria'
                criteria[current_section] = {'count': 0, 'subpoints': {}}
                print(f"Switched to Inclusion_Criteria section. {line} {i}\n")
                
                continue
            elif "Exclusion Criteria" in line or "exclusion Criteria" in line:
                current_section = 'Exclusion_Criteria'
                criteria[current_section] = {'count': 0, 'subpoints': {}}
                print(f"Switched to Exclusion_Criteria section. {line} {i}")
                continue
            else:
                # If the heading is not one of our target headings,
                # it is not considered for processing, so reset section.
                current_section = None
                #print("Heading not relevant to target sections. Skipping...\n")
                continue

        # Process non-heading paragraphs: look for bullet points.
        # bullet_match = re.match(r'^\s*(\d+\.)+\d*[\)\-\.]?\s+', line)
        bullet_match = re.match(r'^\s*(\d+)\.\s+(.*)', line)
        sub_bullet_match = re.match(r'^\s*(\d+)\.(\d+)\s+(.*)', line)



        print(f"lookig at line : {bullet_match}, {line [:30]}")
        test_text = []
        test_text.append(line)
        # print("\n".join(test_text))
        
        if bullet_match and not sub_bullet_match:
            print(f"Entered bullet loop, {line [:30]}")
            main_point = bullet_match.group(1)  # E.g., "1" or "1.1"
            criteria[current_section][count] += 1
            if main_point not in criteria[current_section]["subpoints"]:
                criteria[current_section]["subpoints"][main_point] = 0

            print(f"Detected main bullet : {main_point}")

        elif sub_bullet_match:
            main_point = sub_bullet_match.group(1)
            if main_point in criteria[current_section]["subpoints"]:
                criteria[current_section]["subpoints"][main_point] += 1

            print(f"Detected sub bullet : {main_point}")
            
    
    print("Processing completed.")
    return criteria
