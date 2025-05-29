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
                skip = False
                match_patterns= self.abbr_match_patterns(abbr_clean, abbr, csr_text)
                
                #Check full form of abbr present in paragraph or not
                full_form_found = False
                if abbr_table_dict[abbr].lower() in csr_text.lower(): 
                    full_form_found = True
                
                if match_patterns or full_form_found :
                    match_patterns_pos = self.abbr_match_patterns_pos2(abbr_clean, abbr, csr_text)
                    match_dict = {}
                    counter = 0
#                    break
                    for abbr_pattern in abbr_patterns:
                        if abbr_pattern.format(abbr_dict=abbr_table_dict[abbr], abbr=abbr).lower() in csr_text.lower() or abbr_pattern.format(abbr_dict=self.cleanText_abbr(abbr_table_dict[abbr], removepunc=True), abbr=abbr).lower() in csr_text.lower():
                            counter += 1
                            match_dict[csr_text] = [counter, abbr_pattern]
                            if len(match_dict)>0:
                                match_csr_text = min(match_dict, key=match_dict.get)
                                match_abbr_pattern = match_dict[match_csr_text][1]
                                match_csr_text_abbr = self.find_pos(match_csr_text, match_abbr_pattern.format(abbr_dict=abbr_table_dict[abbr], abbr=abbr))
                                match_csr_text_abbr_pos = self.find_pos2(match_csr_text, match_abbr_pattern.format(abbr_dict=self.cleanText_abbr(abbr_table_dict[abbr], removepunc=True), abbr=abbr))
                                if match_csr_text_abbr=="":
                                    match_csr_text_abbr = self.find_pos(match_csr_text, match_abbr_pattern.format(abbr_dict=self.cleanText_abbr(abbr_table_dict[abbr], removepunc=True), abbr=abbr))
                                
                                pos_match = False
                                try:
                                    pos_match =  int(match_csr_text_abbr_pos[0]) <  int(match_patterns_pos[0])   <   int(match_csr_text_abbr_pos[1]) 
                                except:
                                    pass
                                if abbr not in new_list and pos_match:
                                    #if para has first occurrence and also full form of abbr as second occurrence within
                                    try:
                                        match_pos_till_first_occ = int(match_csr_text_abbr_pos[1])
                                        if csr_text[match_pos_till_first_occ:].count(abbr_table_dict[abbr].lower()) > 0 :
                                            has_first_occ_and_full_form.append(abbr) 
                                    except:
                                        pass                                    
                                    first_occurence.append({'name' : abbr, 'extracted':match_csr_text_abbr, "description":abbr_table_dict[abbr], 'status':'Found',"desc_occurrence":""})
                                    matched=True
                                    new_list.append(abbr)
                                    print(f"new_list: {new_list}")
                                    skip = True
                                    break
                    
                    if not matched  and abbr not in new_list:
                        first_occurence.append({'name' : abbr, 'extracted':"", 'description':abbr_table_dict[abbr], 'status':'Not Found',"desc_occurrence":""})
                        new_list.append(abbr)
                        
                # Add status of those abbr whose description exists post first occurrence exist
                if not skip and abbr in new_list and abbr_table_dict[abbr].lower() in csr_text.lower():
                        description_occurrence.append(abbr)

            if matched == False and abbr not in new_list:
                # Add status of those abbr whose description exists but first occurrence does not exist
                desc_occurrence = "Not Present"
                if description_found:
                    desc_occurrence = "Present"
                    
                    
                first_occurence.append({'name' : abbr, 'extracted':"", 'description':abbr_table_dict[abbr], 'status':'Not Found33',"desc_occurrence":desc_occurrence})
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
        extract_crf = ast.literal_eval(first_occurence)
        print(f"first_occurence at {datetime.now()}: {first_occurence[2]}")
        return first_occurence
