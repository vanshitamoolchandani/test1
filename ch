    def getCSRSections_tables_abbreviation_match_list_check(self, removehead=True):
        header_list = []
        for para in self.document.paragraphs:
            if 'head' in para.style.name.lower() or 'caption:table' in para.style.name.lower():
                header_list.append(para.text.replace(u"\xa0", u" ").replace("\uf0b7", ""))
        if "" in header_list:
            header_list.remove("")

        csr_sections = {}
        csr_multi_check ={}
        table_list = []
        header = ""
        for para in self.getDocument_metadata():
            if ('head' in para.style.name.lower() or 'caption:table' in para.style.name.lower()) and para.text in header_list: #  or 'caption' in para.style.name.lower()
                    if header not in csr_multi_check.keys():
                        csr_multi_check[header] = 0
                        csr_sections[header] = []
                        table_list = [x for x in table_list if x]
                        for x in table_list:
                            first_row = list(x[0].values())
                            if all(v == first_row[0] for v in first_row) and first_row[0] != "" and first_row[0].lower().startswith("table") :
                                #check headers are picked
                                data_df = pd.DataFrame(x[1:])
                                if list(data_df.columns) == list(range(0,len(data_df.columns-1))):
                                    data_df = data_df.rename(columns=data_df.iloc[0]).drop(data_df.index[0])
                                    data_df.columns = [x.replace("\n"," ")for x in list(data_df.columns)]
                                    data = data_df.to_dict(orient='records')
                                    header_new = first_row[0].replace("\t"," ")
                                    if header_new not in csr_multi_check.keys():
                                        csr_multi_check[header_new] = 0
                                    else:
                                        key_table_header = csr_multi_check[header_new]
                                        header_new = header_new + "_" + str(key_table_header)
                                    csr_sections[header_new] = [data]
                                else:
                                   csr_sections[first_row[0].replace("\t"," ")] = [x[1:]]
                            else:
                                csr_sections[header].append(x)
                        header = para.text.replace(u"\xa0", u" ").replace("\uf0b7", "")
                        table_list = []
                    else:
                        csr_multi_check[header] += 1
                        csr_sections[f"{header}_{csr_multi_check[header]}"] = []
                        table_list = [x for x in table_list if x]
                        for x in table_list:
                            first_row = list(x[0].values())
                            if all(v == first_row[0] for v in first_row) and first_row[0] != "" and first_row[0].lower().startswith("table"):
                                data_df = pd.DataFrame(x[1:])
                                if list( data_df.columns) == list(range(0,len(data_df.columns-1))):
                                    data_df = data_df.rename(columns=data_df.iloc[0]).drop(data_df.index[0])
                                    data_df.columns = [x.replace("\n"," ")for x in list(data_df.columns)]
                                    data = data_df.to_dict(orient='records')
                                    header_new = first_row[0].replace("\t"," ")
                                    if header_new not in csr_multi_check.keys():
                                        csr_multi_check[header_new] = 0
                                    else:
                                        key_table_header = csr_multi_check[header_new]
                                        header_new = header_new + "_" + str(key_table_header)
                                    csr_sections[header_new] = [data]
                                else:
                                    csr_sections[first_row[0].replace("\t"," ")] = [x[1:]]
                            else:
                                csr_sections[f"{header}_{csr_multi_check[header]}"].append(x)
                        header = para.text.replace(u"\xa0", u" ").replace("\uf0b7", "")
                        table_list = []
            else:
                #if 'table' in para.style.name.lower():
                if 'table' in str(para.style).lower():
                    try:
                        table_list.append(self.table_extract(para))
#                        table_list.append(para)
                    except:
                        pass

        #for the last header
        if header not in csr_multi_check.keys():
            csr_multi_check[header] = 0
            csr_sections[header] = table_list
            header = para.text.replace(u"\xa0", u" ").replace("\uf0b7", "")
            table_list = []
        else:
            csr_multi_check[header] += 1
            csr_sections[f"{header}_{csr_multi_check[header]}"] = table_list
            header = para.text.replace(u"\xa0", u" ").replace("\uf0b7", "")
            table_list = []
        csr_sections_keys = list(csr_sections.keys())

        if removehead == True:
            for sec in csr_sections_keys:
                if sec == "":
                    del csr_sections[sec]

        ## removing the sections numbers if present before the section heading from csr sections
        try:
            csr_sections = { re.sub(r'^[0-9\.]+\s','',re.sub(r'\n','',k)).strip():v  for k,v in csr_sections.items()}
        except:
            pass

        ##removing the text which contains in b/w << and >>
        csr_sections = {re.sub(r'<<.*?>>','',k):v for k,v in csr_sections.items()}
        return csr_sections
