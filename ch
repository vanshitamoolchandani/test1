def AbbreviationFirstOccurence(self, pt_abbr_list):
        #Extracting the full text from csr by spliting new line
        full_extract = self.getFullExtract_abbr().split("\n")
        abbr_table = self.getAbbrTable(pt_abbr_list)
        abbr_patterns = ['{abbr_dict} {abbr},', '{abbr_dict}({abbr})', '{abbr_dict} ({abbr})', '{abbr_dict}  ({abbr})', '{abbr_dict}s ({abbr})', '{abbr_dict}({abbr}s)', '{abbr_dict} ({abbr}s)', '{abbr_dict}s ({abbr}s)', '{abbr} = {abbr_dict}', '{abbr}={abbr_dict}', '{abbr_dict} [{abbr}]','{abbr_dict} ({abbr}']
        abbr_table_dict = {self.cleanText_abbr(rec['abbreviation']):rec['description'] for rec in abbr_table.to_dict(orient='records')}
        first_occurence=[]
        description_occurrence = []
        has_first_occ_and_full_form = []
                
        for abbr in abbr_table_dict.keys():
            abbr_clean = self.cleanText_abbr(abbr)            
            matched = False
            pos_match = False
            new_list=[]
            foundAbbrList = []
            description_found = False
            for csr_text in full_extract:
                if abbr_table_dict[abbr].lower() in csr_text.lower():
                    description_found = True
                print(f"Checking text: '{csr_text[:30]}'")
                skip = False
                match_patterns= self.abbr_match_patterns(abbr_clean, abbr, csr_text)
                print(f"Found Match: '{match_patterns}'")
                #Check full form of abbr present in paragraph or not
                full_form_found = False
                if abbr_table_dict[abbr].lower() in csr_text.lower(): 
                    full_form_found = True
                print(f"Full form found: '{full_form_found}'")
                
                if match_patterns or full_form_found :
                    match_patterns_pos = self.abbr_match_patterns_pos2(abbr_clean, abbr, csr_text)
                    print(f" match_patterns_pos: '{match_patterns_pos}'")
                    match_dict = {}
                    counter = 0
#                    break
                    for abbr_pattern in abbr_patterns:
                        if abbr_pattern.format(abbr_dict=abbr_table_dict[abbr], abbr=abbr).lower() in csr_text.lower() or abbr_pattern.format(abbr_dict=self.cleanText_abbr(abbr_table_dict[abbr], removepunc=True), abbr=abbr).lower() in csr_text.lower():
                            counter += 1
                            match_dict[csr_text] = [counter, abbr_pattern]
                            print(f"pattern match: '{abbr_pattern}'")
                            if len(match_dict)>0:
                                match_csr_text = min(match_dict, key=match_dict.get)
                                match_abbr_pattern = match_dict[match_csr_text][1]
                                print(f"Matched CSR Text: '{match_csr_text[:30]}'")
                                print(f"Ussing pattern: '{match_abbr_pattern}'")
                                match_csr_text_abbr = self.find_pos(match_csr_text, match_abbr_pattern.format(abbr_dict=abbr_table_dict[abbr], abbr=abbr))
                                match_csr_text_abbr_pos = self.find_pos2(match_csr_text, match_abbr_pattern.format(abbr_dict=self.cleanText_abbr(abbr_table_dict[abbr], removepunc=True), abbr=abbr))
                                if match_csr_text_abbr=="":
                                    match_csr_text_abbr = self.find_pos(match_csr_text, match_abbr_pattern.format(abbr_dict=self.cleanText_abbr(abbr_table_dict[abbr], removepunc=True), abbr=abbr))
                                
                                pos_match = False

                                try:
                                    pos_match =  int(match_csr_text_abbr_pos[0]) <  int(match_patterns_pos[0])   <   int(match_csr_text_abbr_pos[1]) 
                                except:
                                    pass
                                print(f"match_csr_text_abbr_pos: {match_csr_text_abbr_pos}', pos_match: '{pos_match}'")
                                print(f"Append conditions: abbr = {abbr}, in new_list = {abbr in new_list}, pos_match = {pos_match}") 
                                if abbr not in new_list and pos_match:
                                    #if para has first occurrence and also full form of abbr as second occurrence within
                                    try:
                                        match_pos_till_first_occ = int(match_csr_text_abbr_pos[1])
                                        if csr_text[match_pos_till_first_occ:].count(abbr_table_dict[abbr].lower()) > 0 :
                                            has_first_occ_and_full_form.append(abbr) 
                                    except:
                                        pass   
                                    print(f"Appending first occ: abbr = {abbr}, extarcted = {match_csr_text_abbr[:30]}, pos_match = {pos_match}")                                 
                                    first_occurence.append({'name' : abbr, 'extracted':match_csr_text_abbr, "description":abbr_table_dict[abbr], 'status':'Found',"desc_occurrence":""})
                                    matched=True
                                    new_list.append(abbr)
                                    print(f"Added to first occurance(new_list): {abbr}")
                                    skip = True
                                    break
                    
                    if not matched:
                        if abbr not in new_list:
                            first_occurence.append({'name' : abbr, 'extracted':"", 'description':abbr_table_dict[abbr], 'status':'Not Found',"desc_occurrence":""})
                            new_list.append(abbr)
                        print(f"Narked as not found(new_list): {abbr}")
                        
                # Add status of those abbr whose description exists post first occurrence exist
                if not skip and abbr in new_list and abbr_table_dict[abbr].lower() in csr_text.lower():
                        description_occurrence.append(abbr)

            if matched == False and abbr not in new_list:
                # Add status of those abbr whose description exists but first occurrence does not exist
                desc_occurrence = "Not Present"
                if description_found:
                    desc_occurrence = "Present"
                    
                print(f"Adding Not found: abbr = {abbr}, matched = {matched}, in new_list: {new_list}, description_found: {description_found}") 
                first_occurence.append({'name' : abbr, 'extracted':"", 'description':abbr_table_dict[abbr], 'status':'Not Found',"desc_occurrence":desc_occurrence})
                new_list.append(abbr)
                
                
        #Update status of description exist if it has not set
        new_first_occurrence = []
        for each in first_occurence:
            if len(each["desc_occurrence"]) == 0:
                abbreviation = each["name"]
                if abbreviation in description_occurrence or each["name"] in has_first_occ_and_full_form:
                    each["desc_occurrence"] = "Present"
                else:
                    each["desc_occurrence"] = "Not Present"
            new_first_occurrence.append(each)
        
        first_occurence = new_first_occurrence
        print(f"Final output: {first_occurence}")
                
        return first_occurence


Found Match: 'False'
Full form found: 'False'
Checking text: 'Overall, 2725 patients were in'
Found Match: 'False'
Full form found: 'False'
Checking text: 'The key strength of this multi'
Found Match: 'False'
Full form found: 'False'
Checking text: 'Selection bias may be a potent'
Found Match: 'False'
Full form found: 'False'
Checking text: 'The prevalence of MSI-H/dMMR i'
Found Match: 'False'
Full form found: 'False'
Checking text: 'No acknowledgements.'
Found Match: 'False'
Full form found: 'False'
Final output: [{'name': 'AE', 'extracted': '', 'description': 'Adverse events', 'status': 'Not Found', 'desc_occurrence': 'Present'}, {'name': 'CEA', 'extracted': 'carcinoembryonic antigen (CEA)', 'description': 'Carcinoembryonic antigen', 'status': 'Found', 'desc_occurrence': 'Not Present'}, {'name': 'CRF', 'extracted': '', 'description': 'Case Report Form', 'status': 'Not Found', 'desc_occurrence': 'Present'}, {'name': 'dMMR', 'extracted': '', 'description': 'Mismatch Repair Deficient', 'status': 'Not Found', 'desc_occurrence': 'Not Present'}, {'name': 'pMMR', 'extracted': '', 'description': 'Mismatch Repair Proficient', 'status': 'Not Found', 'desc_occurrence': 'Not Present'}, {'name': 'EC', 'extracted': '', 'description': 'Ethics Committee', 'status': 'Not Found', 'desc_occurrence': 'Present'}, {'name': 'ECOG', 'extracted': '', 'description': 'The Eastern Cooperative Oncology Group', 'status': 'Not Found', 'desc_occurrence': 'Not Present'}, {'name': 'EMR', 'extracted': 'electronic medical record (EMR)', 'description': 'Electronic Medical Record', 'status': 'Found', 'desc_occurrence': 'Not Present'}, {'name': 'FAS', 'extracted': 'full analysis set (FAS)', 'description': 'Full Analysis Set', 'status': 'Found', 'desc_occurrence': 'Not Present'}, {'name': 'FFPE', 'extracted': 'Formalin-fixed paraffin-embedded (FFPE)', 'description': 'Formalin-fixed paraffin-embedded', 'status': 'Found', 'desc_occurrence': 'Not Present'}, {'name': 'GPP', 'extracted': '', 'description': 'Good Pharmacoepidemiology Practices', 'status': 'Not Found', 'desc_occurrence': 'Not Present'}, {'name': 'HP', 'extracted': '', 'description': 'Helicobactor Pylori', 'status': 'Not Found', 'desc_occurrence': 'Not Present'}, {'name': 'HPV', 'extracted': '', 'description': 'Human Papilloma Virus', 'status': 'Not Found', 'desc_occurrence': 'Not Present'}, {'name': 'ICF', 'extracted': 'informed consent form (ICF)', 'description': 'Informed Consent Form', 'status': 'Found', 'desc_occurrence': 'Present'}, {'name': 'IEC', 'extracted': 'Independent Ethics Committee (IEC)', 'description': 'Independent Ethics Committee', 'status': 'Found', 'desc_occurrence': 'Not Present'}, {'name': 'IHC', 'extracted': 'immunohistochemistry (IHC)', 'description': 'Immunohistochemistry', 'status': 'Found', 'desc_occurrence': 'Not Present'}, {'name': 'IRB/ERCs', 'extracted': '', 'description': 'Institutional Review Boards/Ethics Review Committees', 'status': 'Not Found', 'desc_occurrence': 'Not Present'}, {'name': 'MSI-H', 'extracted': '', 'description': 'Microsatellite Instability High', 'status': 'Not Found', 'desc_occurrence': 'Not Present'}, {'name': 'MSS', 'extracted': 'microsatellite stability (MSS)', 'description': 'Microsatellite Stability', 'status': 'Found', 'desc_occurrence': 'Not Present'}, {'name': 'NSAR', 'extracted': 'Non-serious adverse reactions (NSARs)', 'description': 'Non-serious adverse reactions', 'status': 'Found', 'desc_occurrence': 'Not Present'}, {'name': 'PCR', 'extracted': 'polymerase chain reaction (PCR)', 'description': 'Polymerase Chain Reaction', 'status': 'Found', 'desc_occurrence': 'Not Present'}, {'name': 'PD-1', 'extracted': '', 'description': 'Programmed Cell Death 1', 'status': 'Not Found', 'desc_occurrence': 'Not Present'}, {'name': 'PD-L1', 'extracted': 'Programmed Death-Ligand 1 (PD-L1)', 'description': 'Programmed Death-Ligand 1', 'status': 'Found', 'desc_occurrence': 'Not Present'}, {'name': 'PQC', 'extracted': 'product quality complaints (PQCs)', 'description': 'Product Quality Complaints', 'status': 'Found', 'desc_occurrence': 'Not Present'}, {'name': 'SAE', 'extracted': '', 'description': 'Serious Adverse event', 'status': 'Not Found', 'desc_occurrence': 'Present'}, {'name': 'SD', 'extracted': '', 'description': 'Standard Deviation', 'status': 'Not Found', 'desc_occurrence': 'Present'}, {'name': 'SOP', 'extracted': 'standard operating procedures (SOPs)', 'description': 'Standard Operating Procedure', 'status': 'Found', 'desc_occurrence': 'Not Present'}, {'name': 'TNM', 'extracted': '', 'description': 'tumor (T), nodes (N), and metastases (M)', 'status': 'Not Found', 'desc_occurrence': 'Not Present'}
