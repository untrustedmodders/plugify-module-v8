import { Plugin } from '|plugify';
const even_or_odd = require('even-or-odd-number');

export class SamplePlugin extends Plugin {
	pluginStart() {
		var number = 2
		console.log(even_or_odd.is_even(number) ? "IS_EVEN" : "IS_ODD");
	}
}
