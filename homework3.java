/*
 * HW3: First-Order Logic Resolution
 * Date: 2017/11/16
 * Grade:
 * Method: Run resolution iteratively until get "" sentence(True) or there is no new clauses be added into KB(False).
 * First, negate the query and add it to KB, put all sentences that contain at least on constant argument and the negated query
 * to "new_sentences" as the first round queries and unify them with the rest of KB. After that, I will iteratively
 * put the new clauses to KB with no duplication and treat them as the next round's queries.
 * 
 * Note: There is no need to resolve every two sentences at each round because the original sentences have already unified with
 * the constant variable sentences at the first round. It is meaningless to resolve two sentences with only variables at the first 
 * round, we will get noting but more undecidable sentences. But after the first round, we should unify every new clause with KB
 * in case miss the right answer. 
 * Example: query:B(John), KB:A(x), ~A(x) | B(John). At first round, we unify ~B(John) and ~A(x) | B(John) get a new clauses ~A(x).
 * At the second round, we unify A(x) and ~A(x) get "", the result is true. If we omit the first round result ~A(x), we have nothing to
 * resolve, and the answer will be wrong
 * 
*/

/*I should delete this package before upload it to Vocareum*/
package homework;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class homework {
	static HashMap<String,HashSet<Integer>> final_predicatemap = new HashMap<String,HashSet<Integer>>(); //predicate -> sentences index map, only for original KB, easy to recoverKB
	static ArrayList<HashMap<String, HashSet<String>>> mainkb = new ArrayList<HashMap<String, HashSet<String>>>(); //maintain a HashMap for each sentence in KB, key is predicate, values are literals themselves, in case one sentence contains different literals with the same predicate
	static int q_num, kb_num; //num of query and num of kb
	static int total_newclauses = 0;//indicate how many new clauses are added in kb, easy to recoverkb
	static int cur_newclauses = 0; //indicate how many new clauses in each round after update kb (delete duplicated ones).
	static int variable_num = 0;//standardize variables, use _number to identify them
	static int final_variable_num = 0; // After finishing one query's resolution, I need to reset variable_num in case it keep growing
	static String[] queries; // query sentences
	static ArrayList<String> kb = new ArrayList<String>(); //kb sentences
	static ArrayList<String> new_sentences = new ArrayList<String>();//new sentences need to be resolved at each round, the old one has been deleted after his part finished (每轮需要搜索kb做resolution的句子，动态更新)
	static ArrayList<HashMap<String,HashSet<String>>> new_cls = new ArrayList<HashMap<String,HashSet<String>>>(); //pack each new sentence as a HashMap, just like mainkb
	static ArrayList<String> final_new_sentences = new ArrayList<String>() ; //store first round new sentences(kb sentences that contain constants) except for query (存储数据库最原始的包含constant的句子，每个query处理完毕后final_new_sentences不变)
	static ArrayList<HashMap<String,HashSet<String>>> final_cls = new ArrayList<HashMap<String,HashSet<String>>>(); //corresponding to final_new_sentences (与final_new_sentences相对应)
	static ArrayList<Integer> constant_sentences_index = new ArrayList<Integer>();//store sentences index that has been unified with specific sentence. During one round unify, I need to make sure that if two sentences are both from new clauses, they should only unify with each other once, and one sentence should not unify with itself
	static HashSet<String> final_check = new HashSet<String>();//maintain kb and abandon duplicated sentences (维护基础kb的check)
	static ArrayList<String> results = new ArrayList<String>();//resolution result for each sentence
	
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		/***************************Read input, get queries and kb****************************/
		ArrayList<String> temp_kb = new ArrayList<String>();//accept kb sentences from input file, but will reorder them into kb, afterthatm temp_kb is meaningless
		try{
			BufferedReader br = new BufferedReader(new InputStreamReader(
					new FileInputStream("input.txt"), "UTF-8"));
			
			q_num = Integer.parseInt(br.readLine());
			queries = new String[q_num];
			for(int i=0; i<q_num;i++){
				queries[i] = br.readLine();
			}
			
			kb_num = Integer.parseInt(br.readLine());
			
			for(int i=0; i<kb_num;i++){
				temp_kb.add(br.readLine());
			}
		}catch(Exception ex){
			ex.printStackTrace();
		}
		
		/*************************************************************************************/
		/* 
		 * pack nonduplicated kb sentences to mainkb and kb, maintain final_* things 
		 * screen all sentences with constant argument and add them to final_new_sentences, aka first round new clauses
		 * standardize all variables using _index and uppercase all constant arguments
		*/
		/*************************************************************************************/
		transferToHashMap(temp_kb);
		
		/*************************************************************************************/
		/*
		 * standardize queries with the same policy as kb
		 */
		/*************************************************************************************/
		standardizequery(queries);
		
		for(int i=0; i<q_num;i++){
			
			resolution(queries[i]);
			recoverkb();
			
			System.out.println(i + "finished!");
		}
		
		FileWriter out=null;
		try {
			out=new FileWriter("output.txt");
		} catch (IOException e1) {
			e1.printStackTrace();
		}
		
		try {
			for(int i=0;i<results.size();i++){
				out.write(results.get(i));
				out.write("\r\n");
			}
			out.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	/* 
	 * pack nonduplicated kb sentences to mainkb and kb, maintain final_* things 
	 * screen all sentences with constant argument and add them to final_new_sentences, aka first round new clauses
	 * standardize all variables using _index and uppercase all constant arguments
	*/
	
	public static void transferToHashMap(ArrayList<String> temp_kb){
		for(int l=0; l<temp_kb.size();l++){
			String rule = temp_kb.get(l).replaceAll("\\s", "");
			String dnf[] = rule.split("\\|");
			String new_def = "";
			HashMap<String, HashSet<String>> map = new HashMap<String, HashSet<String>>();
			boolean flag_variable = false; //whether a sentence contains variable
			boolean flag_constant = false; //whether a sentence contains constant
			for(int i=0; i<dnf.length;i++){
				String temp = dnf[i];
				char[] str = new char[temp.length()];
				str = temp.toCharArray();
				int k=0;
				String key = "";
				String variables = "";
				HashSet<String> list;
				HashSet<Integer> kb_index;
				while(str[k]!='('){
					key = key+str[k];
					k++;
				}
				k++;
				while(str[k]!=')'){
					variables = variables+str[k];
					k++;
				}
				//将所有constant变成全部大写
				String variable_split[] = variables.split("\\,");
				for(int j=0;j<variable_split.length;j++){
					if(Character.isUpperCase(variable_split[j].charAt(0))){
						flag_constant = true;
						temp = temp.replaceAll(variable_split[j], variable_split[j].toUpperCase());
					}
					//将constant中的数字也换成constant的首字母，避免与变量的数字变换混淆
					Pattern p = Pattern.compile(".*\\d+.*");
					Matcher m = p.matcher(variable_split[j]);
					if(m.matches()){
						String str1 = variable_split[j].replaceAll("\\d+",""+variable_split[j].charAt(0));
						temp = temp.replaceAll(variable_split[j], str1);
					}
				}
				//将所有变量standardize,constant全变成大写之后就不会有Liz受z影响的问题了，所以一定要先变大写在标准化。
				for(int j=0;j<variable_split.length;j++){
					if(Character.isLowerCase(variable_split[j].charAt(0))){
						temp = temp.replaceAll(variable_split[j], variable_split[j]+final_variable_num);
						flag_variable = true;
					}
				}
				new_def = new_def==""?temp:new_def + " | " + temp;
				
				if(!map.containsKey(key)){
					list = new HashSet<String>();
					list.add(temp);
					map.put(key, list);
				}
				else{
					list = map.get(key);
					list.add(temp);
				}
				if(!final_predicatemap.containsKey(key)){
					kb_index = new HashSet<Integer>();
					kb_index.add(l);
					final_predicatemap.put(key, kb_index);
				}
				else{
					kb_index = final_predicatemap.get(key);
					kb_index.add(l);
				}
			}
			if(flag_variable == true){
				final_variable_num++;
				flag_variable = false;
			}
			kb.add(new_def);
			mainkb.add(map);
			final_check.add(new_def);
			
			if(flag_constant == true){
				final_new_sentences.add(new_def);
				final_cls.add(map);
				constant_sentences_index.add(l);
				flag_constant = false;
			}	
			
		}		
	}
	
	/*
	 * standardize queries with the same policy as kb
	 */
	
	public static void standardizequery(String[] queries){
		for(int i=0; i<queries.length; i++){			
			char[] token = new char[queries[i].length()];
			token = queries[i].toCharArray();
			String q_variable="";
			String q_variable_split[];
			for(int temp =0; temp<token.length;temp++){
				if(token[temp] == '('){
					temp++;
					while(token[temp]!=')'){
						q_variable+=token[temp++];
					}
				}
			}
			q_variable_split = q_variable.split("\\,");
			for(int temp=0; temp<q_variable_split.length; temp++){
				if(Character.isUpperCase(q_variable_split[temp].charAt(0))){
					queries[i] = queries[i].replaceAll(q_variable_split[temp], q_variable_split[temp].toUpperCase());
				}
				//将constant中的数字也换成constant的首字母，避免与变量的数字变换混淆
				Pattern p = Pattern.compile(".*\\d+.*");
				Matcher m = p.matcher(q_variable_split[temp]);
				if(m.matches()){
					String str1 = q_variable_split[temp].replaceAll("\\d+",""+q_variable_split[temp].charAt(0));
					queries[i] = queries[i].replaceAll(q_variable_split[temp], str1);
				}
			}
		}
	}
	
	/*
	 * resolve the query and put "True" or "False" into results
	 */
	public static void resolution(String query){
		/*********************************************Pre-processing, synchronize them with kb ****************************************/
		
		variable_num = final_variable_num;
		Set<String> check = new HashSet<String>(final_check);
		HashMap<String,HashSet<Integer>> predicatemap = new HashMap<String,HashSet<Integer>>();
		Iterator<HashMap.Entry<String, HashSet<Integer>>> it = final_predicatemap.entrySet().iterator();
		while(it.hasNext()){
			String key = it.next().getKey();
			predicatemap.put(key, new HashSet<Integer>(final_predicatemap.get(key)));
		}
		for(int t=0;t<final_new_sentences.size();t++){
			new_sentences.add(final_new_sentences.get(t));
			new_cls.add(final_cls.get(t));
		}
		
		/******************************************************************************************************************************/
		
		boolean flag; //indicate whether there exist new sentence add to kb
		
		/*
		 * turn query into contradiction form and add it to kb
		 */
		if(query.charAt(0) == '~')
			query = query.substring(1,query.length());
		else{
			query = '~'+query;
		}
		
		HashMap<String, HashSet<String>> map = new HashMap<String, HashSet<String>>();
		char[] str = new char[query.length()];
		str = query.toCharArray();
		int k=0;
		String key = "";
		HashSet<String> list = new HashSet<String>();
		HashSet<Integer> kb_index;
		while(str[k]!='('){
			key = key+str[k];
			k++;
		}
		list.add(query);
		map.put(key, list);
		
		new_sentences.add(query);	
		new_cls.add(map);
		kb.add(query);
		mainkb.add(map);
		check.add(query);
		constant_sentences_index.add(kb.size()-1);
		total_newclauses++;
		if(!predicatemap.containsKey(key)){
			kb_index = new HashSet<Integer>();
			kb_index.add(kb.size()-1);
			predicatemap.put(key, kb_index);
		}
		else{
			kb_index = predicatemap.get(key);
			kb_index.add(kb.size()-1);
		}
		
		/*
		 * In the first round, all sentences with constant argument should unify with other sentences
		 */
		cur_newclauses = new_sentences.size();
		/***********************************************************************************/
		
		//store the index of sentences that the query has been unified and itself
		ArrayList<HashSet<Integer>> first_non_duplicate_unify = new ArrayList<HashSet<Integer>>(cur_newclauses);
		for(int temp = 0; temp < cur_newclauses; temp++){
			first_non_duplicate_unify.add(new HashSet<Integer>());
		}
		for(int i=0;i<cur_newclauses;i++){
			//每次resolve new_cls中的一个句子，结果存储在new_sentences中，删除new_cls推演过的句子
			Map<String,Set<String>> querymap = new HashMap<String,Set<String>>(new_cls.get(0));
			new_cls.remove(0);
			resolve(first_non_duplicate_unify, predicatemap, querymap,i,constant_sentences_index.get(i));
			new_sentences.remove(0);
			//new_cls has been cleared
			if(new_sentences.contains("")){
				results.add("TRUE");
				return;
			}
		}
		
		/*
		 * put new sentences into kb and abandon duplicate ones
		 * */
		flag =updatekb(predicatemap, check);
		total_newclauses+=new_sentences.size();
		
		/*
		 * the condition to stop iteration should be 
		 * 1. get "" -> true
		 * 2. no new sentences -> false
		 * 3. total_sentences >maximum_sentences -> false (the new clauses will keep growing and never stop, it is false generally
		 * 4. new_clauses > 1000 -> false (same as 3)
		 */
		int maximum_sentences = Math.max(10*kb_num, 20000);
		while(kb.size()<maximum_sentences){
			
			cur_newclauses = new_sentences.size();
			if(cur_newclauses>10000){
				break;
			}
			
			/***********************************************************************************/
			ArrayList<HashSet<Integer>> non_duplicate_unify = new ArrayList<HashSet<Integer>>(cur_newclauses);
			for(int temp = 0; temp < cur_newclauses; temp++){
				non_duplicate_unify.add(new HashSet<Integer>());
			}
			/*************************************************************************************/
			for(int i=0;i<cur_newclauses;i++){
				//Every time, resolve one sentence of new_cls and delete it after finishing resolution
				Map<String,Set<String>> querymap = new HashMap<String,Set<String>>(new_cls.get(0));
				new_cls.remove(0);
				
				resolve(non_duplicate_unify, predicatemap, querymap,i);
				
				new_sentences.remove(0);
				//new_cls has been cleared
				if(new_sentences.contains("")){
					results.add("TRUE");
					return;
				}
			}
			//update kb, check whether there is new sentence or not
			flag =updatekb(predicatemap, check);
			total_newclauses+=new_sentences.size();
			
			//no new sentences, the answer is false
			if(flag == false){
				results.add("FALSE");
				return;
			}
			//否则继续循环			
		}
		results.add("FALSE");		
	}
	
	/*
	 * For the first round
	 * Give a query, unify it with kb, get all possible new sentences and add them to new_clauses
	 * */
	private static void resolve(ArrayList<HashSet<Integer>> non_duplicate_unify, HashMap<String,HashSet<Integer>> predicatemap, Map<String,Set<String>> querymap, int query_index,int actural_index){
		
		//query cannot unify with itself
		non_duplicate_unify.get(query_index).add(actural_index);

		/*
		 * Give a new query, resolve one predicate at a time, one predicate maybe correspond to many literals, resolve one literal at a time
		 * */
		for(Entry<String, Set<String>> entry : querymap.entrySet()){
			String predicate = entry.getKey();
			//queries是一个predicate(map.key)下所有的literal
			Set<String> queries = entry.getValue();
			
			//resolve one literal at a time
			for(String query : queries){
				char[] token = new char[query.length()];
				token = query.toCharArray();
				String q_variable="";
				String q_variable_split[];
				for(int temp =0; temp<token.length;temp++){
					if(token[temp] == '('){
						temp++;
						while(token[temp]!=')'){
							q_variable+=token[temp++];
						}
					}
				}
				//kb and kb unify，if one literal has no constant, skip
				Pattern p = Pattern.compile(".*[A-Z].*");
				Matcher m = p.matcher(q_variable);
				if(!m.matches()){
					continue;
				}
				q_variable_split = q_variable.split("\\,");
				HashSet<Integer> potential_sentences = new HashSet<Integer>(); //所有句子的下标
				String potential_key; //on the opposite of predicate
				if(predicate.charAt(0) == '~'){
					potential_key = predicate.substring(1,predicate.length());
					 
				}
				else{
					potential_key = '~'+predicate;
				}
				
				// get all index of sentences that maybe could unify with the query
				potential_sentences = predicatemap.get(potential_key);
				if(potential_sentences == null){
					return;
				}
				Iterator<Integer> iter = potential_sentences.iterator();  
				while(iter.hasNext()){	
					int index = iter.next();
					//判断要unify的句子是否与该句子是同级的，如果是要去除重复unify的可能
					if(non_duplicate_unify.get(query_index).contains(index)){
						continue;
					}
					//没有跟其unify过，那么要在对方的HashSet上记上一笔，避免重复
					else{
						if(constant_sentences_index.contains(index)){
							int unified_sentence_index=constant_sentences_index.indexOf(index);
							non_duplicate_unify.get(unified_sentence_index).add(actural_index);
						}						
					}
					//get one potential sentence
					Map<String,Set<String>> kb_sentences = new HashMap<String,Set<String>>(mainkb.get(index));
					//there maybe more than one literal that has the same potential_key
					Set<String>unified_sentences = new HashSet <String>(kb_sentences.get(potential_key));
										
					String kb_sentence = null;
					if(unified_sentences!=null){
						outer:
						for(Iterator<String> it = unified_sentences.iterator();it.hasNext();){
							String variable="";
							String variable_split[] = null;
							kb_sentence = it.next();
							token = new char[kb_sentence.length()];
							token = kb_sentence.toCharArray();
							
							for(int temp =0; temp<token.length;temp++){
								if(token[temp] == '('){
									temp++;
									while(token[temp]!=')'){
										variable+=token[temp++];
									}
								}
							}
							variable_split = variable.split("\\,");
							
							if(variable_split!=null && (q_variable_split.length == variable_split.length)){
								Map<String,String> unify_results = new HashMap<String, String>();
								//对应位置unify
								boolean success = unification(q_variable_split, variable_split, unify_results);
								
								if(success == true){
									
									//first merge the two sentences together and then replace variables with unification results
									String sentence_1 = new_sentences.get(0);
									if(querymap.size()==1 && queries.size()==1){
										sentence_1 = "";
									}
									else{
										sentence_1 = sentence_1.replaceAll("\\s", "");
										String[]temp = sentence_1.split("\\|");
										sentence_1 ="";
										for(int t=0;t<temp.length;t++){
											if(temp[t].equals(query)){
												temp[t] = "";
											}
											if(temp[t].equals("")){
												continue;
											}
											else{
												sentence_1 = sentence_1.equals("")?temp[t]:sentence_1 + " | " + temp[t];
											}
										}
										
									}							
									String sentence_2 = kb.get(index);
									if(kb_sentences.size()==1 && unified_sentences.size()==1){
										sentence_2 = "";
									}
									else{
										sentence_2 = sentence_2.replaceAll("\\s", "");
										String[]temp = sentence_2.split("\\|");
										sentence_2 ="";
										for(int t=0;t<temp.length;t++){
											if(temp[t].equals(kb_sentence)){
												temp[t] = "";
											}
											if(temp[t].equals("")){
												continue;
											}
											else{
												sentence_2 = sentence_2.equals("")?temp[t]: sentence_2 + " | " + temp[t];
											}
										}
									}
																
									String new_sentence = sentence_1.equals("")?sentence_2:sentence_2.equals("")?sentence_1:sentence_1+ " | " + sentence_2;
									if(!sentence_1.equals("")){
										for(int k=0;k<q_variable_split.length;k++){
											String subsitute = unify_results.get(q_variable_split[k]);
											//有subsitute
											if(subsitute!=null){
												String regex = "\\b"+q_variable_split[k]+"\\b";											
												new_sentence = new_sentence.replaceAll(regex, subsitute);
											}
										}
									}
									if(!sentence_2.equals("")){
										for(int k=0;k<variable_split.length;k++){
											String subsitute = unify_results.get(variable_split[k]);
											if(subsitute!=null){
												String regex = "\\b"+variable_split[k]+"\\b";
												new_sentence = new_sentence.replaceAll(regex, subsitute);
											}
											
										}
									}
									
									//最后将所有变量的_index标识更新，以免和kb中原有句子混淆
									Pattern p1 = Pattern.compile(".*\\d+.*");
									Matcher m1 = p1.matcher(new_sentence);

									if (m1.matches()){
										new_sentence = new_sentence.replaceAll("\\d+", ""+variable_num++);	
									}							
									new_sentences.add(new_sentence);
								}
							}							
						}	
					}													
				}
			}
		}
	}
	
	/*
	 * For the rest of the rounds
	 * Give a query, unify it with kb, get all possible new sentences and add them to new_clauses
	 * */
	private static void resolve(ArrayList<HashSet<Integer>> non_duplicate_unify, HashMap<String,HashSet<Integer>> predicatemap, Map<String,Set<String>> querymap, int query_index){
		//resolve constant_sentence中的一个句子，ind_new对应句子的下标，querymap是整个句子的map表达
		int kb_total_size = kb.size();
		int new_kb_startpos = kb_total_size - cur_newclauses;
		//query不能跟自己Unify
		non_duplicate_unify.get(query_index).add(new_kb_startpos+query_index);

		for(Entry<String, Set<String>> entry : querymap.entrySet()){
			String predicate = entry.getKey();
			//queries是一个predicate(map.key)下所有的literal
			Set<String> queries = entry.getValue();
			//query是相同predicate下的一个Literal
			for(String query : queries){
				char[] token = new char[query.length()];
				token = query.toCharArray();
				String q_variable="";
				String q_variable_split[];
				for(int temp =0; temp<token.length;temp++){
					if(token[temp] == '('){
						temp++;
						while(token[temp]!=')'){
							q_variable+=token[temp++];
						}
					}
				}
				q_variable_split = q_variable.split("\\,");
				HashSet<Integer> potential_sentences = new HashSet<Integer>(); //所有句子的下标
				String potential_key;
				if(predicate.charAt(0) == '~'){
					potential_key = predicate.substring(1,predicate.length());
					 
				}
				else{
					potential_key = '~'+predicate;
				}
				potential_sentences = predicatemap.get(potential_key);
				if(potential_sentences == null){
					return;
				}
				Iterator<Integer> iter = potential_sentences.iterator();  
				while(iter.hasNext()){	
					int index = iter.next();
					//判断要unify的句子是否与该句子是同级的，如果是要去除重复unify的可能
					if(index >= new_kb_startpos){
						//检查一下别人是否已经与自己unify过了，如果是，跳过该句子
						if(non_duplicate_unify.get(query_index).contains(index)){
							continue;
						}
						//没有跟其unify过，那么要在对方的HashSet上记上一笔，避免重复
						else{
							non_duplicate_unify.get(index-new_kb_startpos).add(query_index + new_kb_startpos);
						}
					}
					Map<String,Set<String>> kb_sentences = new HashMap<String,Set<String>>(mainkb.get(index));
					Set<String>unified_sentences = new HashSet <String>(kb_sentences.get(potential_key));
										
					String kb_sentence = null;
					if(unified_sentences!=null){
						outer:
						for(Iterator<String> it = unified_sentences.iterator();it.hasNext();){
							String variable="";
							String variable_split[] = null;
							kb_sentence = it.next();
							token = new char[kb_sentence.length()];
							token = kb_sentence.toCharArray();
							
							for(int temp =0; temp<token.length;temp++){
								if(token[temp] == '('){
									temp++;
									while(token[temp]!=')'){
										variable+=token[temp++];
									}
								}
							}
							variable_split = variable.split("\\,");
							
							if(variable_split!=null && (q_variable_split.length == variable_split.length)){
								Map<String,String> unify_results = new HashMap<String, String>();
								boolean success = unification(q_variable_split, variable_split, unify_results);
								if(success == true){
									//加入新句子
									String sentence_1 = new_sentences.get(0);
									if(querymap.size()==1 && queries.size()==1){
										sentence_1 = "";
									}
									else{
										sentence_1 = sentence_1.replaceAll("\\s", "");
										String[]temp = sentence_1.split("\\|");
										sentence_1 ="";
										for(int t=0;t<temp.length;t++){
											if(temp[t].equals(query)){
												temp[t] = "";
											}
											if(temp[t].equals("")){
												continue;
											}
											else{
												sentence_1 = sentence_1.equals("")?temp[t]:sentence_1 + " | " + temp[t];
											}
										}
										
									}							
									String sentence_2 = kb.get(index);
									if(kb_sentences.size()==1 && unified_sentences.size()==1){
										sentence_2 = "";
									}
									else{
										sentence_2 = sentence_2.replaceAll("\\s", "");
										String[]temp = sentence_2.split("\\|");
										sentence_2 ="";
										for(int t=0;t<temp.length;t++){
											if(temp[t].equals(kb_sentence)){
												temp[t] = "";
											}
											if(temp[t].equals("")){
												continue;
											}
											else{
												sentence_2 = sentence_2.equals("")?temp[t]: sentence_2 + " | " + temp[t];
											}
										}
									}
																
									String new_sentence = sentence_1.equals("")?sentence_2:sentence_2.equals("")?sentence_1:sentence_1+ " | " + sentence_2;
									if(!sentence_1.equals("")){
										for(int k=0;k<q_variable_split.length;k++){
											String subsitute = unify_results.get(q_variable_split[k]);
											//有subsitute
											if(subsitute!=null){
												String regex = "\\b"+q_variable_split[k]+"\\b";											
												new_sentence = new_sentence.replaceAll(regex, subsitute);
											}
										}
									}
									if(!sentence_2.equals("")){
										for(int k=0;k<variable_split.length;k++){
											String subsitute = unify_results.get(variable_split[k]);
											if(subsitute!=null){
												String regex = "\\b"+variable_split[k]+"\\b";
												new_sentence = new_sentence.replaceAll(regex, subsitute);
											}
											
										}
									}
									Pattern p = Pattern.compile(".*\\d+.*");
									Matcher m = p.matcher(new_sentence);

									if (m.matches()){
										new_sentence = new_sentence.replaceAll("\\d+", ""+variable_num++);	
									}							
									new_sentences.add(new_sentence);
								}
							}							
						}	
					}													
				}
			}
		}
	}
	
	//对应位置unify
	private static boolean unification(String[] q_variable_split, String[] variable_split,Map<String,String> unify_results){
		boolean flag;
		for(int i=0;i<variable_split.length;i++){
			flag = unify(q_variable_split[i],variable_split[i],unify_results);
			if(flag == false){
				return false;
			}
		}
		return true;
	}
	
	//如果某个变量已经有了unify结果，那么就拿该结果替代变量继续Unify，目标是越统一越好
	private static boolean unify(String q_variable,String variable , Map<String,String>unify_results){
		String subsitute;
		boolean flag = false;
		if(q_variable.equals(variable)){
			return true;
		}
		//判断双方是否是variable
		if(Character.isLowerCase(q_variable.charAt(0))){
			if((subsitute = unify_results.get(q_variable))!=null){
				flag = unify(subsitute,variable,unify_results);
			}
			else if((subsitute = unify_results.get(variable)) !=null){
				flag = unify(q_variable,subsitute,unify_results);
			}
			else{
				unify_results.put(q_variable, variable);
				flag = true;
			}
		}
		
		else if(Character.isLowerCase(variable.charAt(0))){
			if((subsitute = unify_results.get(variable))!=null){
				flag = unify(subsitute,q_variable,unify_results);
			}
			else if((subsitute = unify_results.get(q_variable)) !=null){
				flag = unify(variable,subsitute,unify_results);
			}
			else{
				unify_results.put(variable, q_variable);
				flag = true;
			}
		}
		return flag;
	}
	
	//If no new clause could be added, return false
	private static boolean updatekb(HashMap<String,HashSet<Integer>> predicatemap, Set<String> check){
		//至少有一个新句子，flag变为true
		ArrayList<String> helper = new ArrayList<String>(new_sentences);
		new_sentences.clear();
		boolean flag = false;
		for(int l=0;l<helper.size();l++){
			String rule = helper.get(l);
			HashSet<String> non_dulplicate_key = new HashSet<String>();
			String check_duplicate_rule = rule.replaceAll("\\s", "");
			String dnf[] = check_duplicate_rule.split("\\|");
			String non_duplicate_rule = "";
			String variables="";
			HashMap<String, HashSet<String>> map = new HashMap<String, HashSet<String>>();
			HashSet<String> duplicate_check = new HashSet<String>();
			for(int i=0; i<dnf.length;i++){
				String temp = dnf[i];
				//一句话中可能有重复的句子；
				if(duplicate_check.contains(temp)){
					continue;
				}
				duplicate_check.add(temp);
				non_duplicate_rule = non_duplicate_rule.equals("")?temp:non_duplicate_rule + " | " + temp;
				char[] str = new char[temp.length()];
				str = temp.toCharArray();
				int k=0;
				String key = "";
				HashSet<String> list;
				
				while(str[k]!='('){
					key = key+str[k];
					k++;
				}
				non_dulplicate_key.add(key);
				k++;
				while(str[k]!=')'){
					variables = variables+str[k];
					k++;
				}
				if(!map.containsKey(key)){
					list = new HashSet<String>();
					list.add(temp);
					map.put(key, list);
				}
				else{
					list = map.get(key);
					flag = list.add(temp);
				}
			
			}
			if(check.contains(non_duplicate_rule)){
				continue;
			}
			flag = true;
			check.add(non_duplicate_rule);
			
			new_sentences.add(non_duplicate_rule);
			new_cls.add(map);			
			kb.add(non_duplicate_rule);
			mainkb.add(map);
			
			HashSet<Integer> kb_index;
			Iterator<String> it = non_dulplicate_key.iterator();
			while(it.hasNext()){
				String key = it.next();
				if(predicatemap.containsKey(key)){
					kb_index = predicatemap.get(key);
					kb_index.add(kb.size()-1);
				}
				else{
					kb_index = new HashSet<Integer>();
					kb_index.add(kb.size()-1);
					predicatemap.put(key, kb_index);
				}
			}
		
		}
		return flag;
	}

	//after finish one query, recover the kb to the original one
	public static void recoverkb(){		
		while(total_newclauses>1){
			kb.remove(kb.size()-1);			
			mainkb.remove(mainkb.size()-1);
			total_newclauses--;
		}
		constant_sentences_index.remove(constant_sentences_index.size()-1);
		kb.remove(kb.size()-1);			
		mainkb.remove(mainkb.size()-1);
		total_newclauses--;
		new_sentences.clear();
		new_cls.clear();
	}
	
}
